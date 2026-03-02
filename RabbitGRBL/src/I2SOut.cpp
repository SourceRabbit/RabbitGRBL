/*
    I2SOut.cpp

    Copyright (c) 2026 Nikolaos Siatras
    Twitter: nsiatras
    Github: https://github.com/nsiatras
    Website: https://www.sourcerabbit.com

    Drives a 74HC595 shift register via the ESP32 I2S0 peripheral + DMA.
    Each of the 32 output bits maps to one virtual stepper pin (STEP/DIR/DISABLE).

    Rabbit GRBL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rabbit GRBL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Grbl.h"

#ifdef USE_I2S_STEPS
#include "I2SOut.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/periph_ctrl.h>
#include <driver/gpio.h>
#include <rom/lldesc.h>
#include <soc/i2s_struct.h>
#include <soc/i2s_reg.h>
#include <esp_intr_alloc.h>
#include <atomic>
#include <string.h>

// ---------------------------------------------------------------------------
// Internal constants
// ---------------------------------------------------------------------------
static const int I2S_SAMPLE_SIZE = 4; // bytes per 32-bit DMA sample
static const int DMA_SAMPLE_COUNT = I2S_OUT_DMABUF_LEN / I2S_SAMPLE_SIZE;

// How many samples to keep free at the end of a buffer to avoid overrun
static const int SAMPLE_SAFE_COUNT = (20 / I2S_OUT_USEC_PER_PULSE);

// ---------------------------------------------------------------------------
// DMA ring buffer bookkeeping
// ---------------------------------------------------------------------------
typedef struct
{
    uint32_t **fBuffers; // Array of DMA data buffers (one per descriptor)
    uint32_t *fCurrent;  // Pointer to the buffer currently being filled by the task
    uint32_t fRwPos;     // Write position within fCurrent (in samples)
    lldesc_t **fDesc;    // Array of DMA descriptors (ring-linked)
    xQueueHandle fQueue; // ISR -> task notification queue (passes finished descriptor)
} I2sOutDma;

// ---------------------------------------------------------------------------
// Module-level state
// ---------------------------------------------------------------------------
static I2sOutDma gDma;
static intr_handle_t gIsrHandle;
static int gInitialized = 0;

// Shadow register: holds the current logical state of all 32 virtual pins
static std::atomic<uint32_t> gPortData(0);

// Pulser state machine
static volatile I2sOutPulserStatus gPulserStatus = PASSTHROUGH;

// Spinlocks
static portMUX_TYPE gSpinlock = portMUX_INITIALIZER_UNLOCKED;
static portMUX_TYPE gPulserSpinlock = portMUX_INITIALIZER_UNLOCKED;

// ---------------------------------------------------------------------------
// Forward declarations of internal helpers
// ---------------------------------------------------------------------------
static void i2sOutStart();
static void i2sOutStop();
static int IRAM_ATTR clearDmaBuffer(lldesc_t *desc, uint32_t val);
static int IRAM_ATTR clearAllDmaBuffers(uint32_t val);
static void IRAM_ATTR i2sIsrHandler(void *arg);
static void IRAM_ATTR i2sOutTask(void *param);

// ---------------------------------------------------------------------------
// Helper: route a GPIO through the I2S output signal matrix
// ---------------------------------------------------------------------------
static void gpioMatrixOutSet(uint8_t gpio, uint32_t signalIdx)
{
    if (gpio == 255)
    {
        return;
    }
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t)gpio, (gpio_mode_t)GPIO_MODE_DEF_OUTPUT);
    gpio_matrix_out(gpio, signalIdx, false, false);
}

// ---------------------------------------------------------------------------
// Helper: push current port data to the I2S single-data register (PASSTHROUGH)
// ---------------------------------------------------------------------------
static inline void IRAM_ATTR i2sOutSingleData()
{
    I2S0.conf_single_data = gPortData.load(); // C++ atomic method
}

// ---------------------------------------------------------------------------
// Clear one DMA buffer with a constant 32-bit value
// ---------------------------------------------------------------------------
static int IRAM_ATTR clearDmaBuffer(lldesc_t *desc, uint32_t val)
{
    uint32_t *buf = (uint32_t *)desc->buf;
    for (int i = 0; i < DMA_SAMPLE_COUNT; i++)
    {
        buf[i] = val;
    }
    desc->length = I2S_OUT_DMABUF_LEN;
    return 0;
}

// ---------------------------------------------------------------------------
// Clear all DMA buffers and re-link descriptors into a ring
// ---------------------------------------------------------------------------
static int IRAM_ATTR clearAllDmaBuffers(uint32_t val)
{
    for (int i = 0; i < I2S_OUT_DMABUF_COUNT; i++)
    {
        gDma.fDesc[i]->owner = 1;
        gDma.fDesc[i]->eof = 1;
        gDma.fDesc[i]->sosf = 0;
        gDma.fDesc[i]->length = I2S_OUT_DMABUF_LEN;
        gDma.fDesc[i]->size = I2S_OUT_DMABUF_LEN;
        gDma.fDesc[i]->buf = (uint8_t *)gDma.fBuffers[i];
        gDma.fDesc[i]->offset = 0;

        // Ring-link: last descriptor wraps back to the first
        gDma.fDesc[i]->qe.stqe_next = (lldesc_t *)((i < I2S_OUT_DMABUF_COUNT - 1)
                                                       ? gDma.fDesc[i + 1]
                                                       : gDma.fDesc[0]);
        clearDmaBuffer(gDma.fDesc[i], val);
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Start I2S TX with DMA
// ---------------------------------------------------------------------------
static void i2sOutStart()
{
    portENTER_CRITICAL(&gSpinlock);

    // Reset TX FIFO and DMA
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
    I2S0.lc_conf.out_rst = 1;
    I2S0.lc_conf.out_rst = 0;

    // Point DMA to the first descriptor
    I2S0.out_link.addr = (uint32_t)gDma.fDesc[0];
    I2S0.out_link.start = 1;
    I2S0.fifo_conf.dscr_en = 1;
    I2S0.conf.tx_start = 1;

    portEXIT_CRITICAL(&gSpinlock);
}

// ---------------------------------------------------------------------------
// Stop I2S TX
// ---------------------------------------------------------------------------
static void i2sOutStop()
{
    portENTER_CRITICAL(&gSpinlock);
    I2S0.out_link.stop = 1;
    I2S0.conf.tx_start = 0;
    I2S0.fifo_conf.dscr_en = 0;
    portEXIT_CRITICAL(&gSpinlock);
}

// ---------------------------------------------------------------------------
// DMA ISR: fires when a descriptor's EOF bit is reached
// ---------------------------------------------------------------------------
static void IRAM_ATTR i2sIsrHandler(void *arg)
{
    portBASE_TYPE highPriorityAwoken = pdFALSE;

    if (I2S0.int_st.out_eof || I2S0.int_st.out_total_eof)
    {
        if (I2S0.int_st.out_total_eof)
        {
            // End of DMA chain — stop the transmitter
            portENTER_CRITICAL_ISR(&gSpinlock);
            I2S0.out_link.stop = 1;
            I2S0.fifo_conf.dscr_en = 0;
            I2S0.conf.tx_start = 0;
            portEXIT_CRITICAL_ISR(&gSpinlock);
        }

        lldesc_t *finishedDesc = (lldesc_t *)I2S0.out_eof_des_addr;

        // If the queue is full (task is too slow), discard oldest entry
        if (xQueueIsQueueFullFromISR(gDma.fQueue))
        {
            lldesc_t *staleDesc;
            xQueueReceiveFromISR(gDma.fQueue, &staleDesc, &highPriorityAwoken);
            // Fill the evicted buffer with the current idle state
            uint32_t *buf = (uint32_t *)staleDesc->buf;
            uint32_t val = gPortData.load(); // C++ atomic method
            for (int i = 0; i < DMA_SAMPLE_COUNT; i++)
            {
                buf[i] = val;
            }
            staleDesc->length = I2S_OUT_DMABUF_LEN;
        }

        // Notify the fill task
        xQueueSendFromISR(gDma.fQueue, &finishedDesc, &highPriorityAwoken);
    }

    if (highPriorityAwoken == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }

    // Clear all pending interrupt flags
    I2S0.int_clr.val = I2S0.int_st.val;
}

// ---------------------------------------------------------------------------
// DMA fill task: refills each DMA buffer as it completes
// ---------------------------------------------------------------------------
static void IRAM_ATTR i2sOutTask(void *param)
{
    lldesc_t *desc;
    while (1)
    {
        // Block until the ISR signals a buffer completion
        xQueueReceive(gDma.fQueue, &desc, portMAX_DELAY);

        portENTER_CRITICAL(&gPulserSpinlock);

        gDma.fCurrent = (uint32_t *)desc->buf;
        gDma.fRwPos = 0;

        if (gPulserStatus == STEPPING)
        {
            // Fill with idle-level port data (actual step pulses are pushed
            // directly by Stepper_I2S::Step()/Unstep() via i2sOutPushSample)
            uint32_t portData = gPortData.load(); // C++ atomic method
            uint32_t *buf = (uint32_t *)desc->buf;
            for (int i = 0; i < DMA_SAMPLE_COUNT; i++)
            {
                buf[i] = portData;
            }
            desc->length = I2S_OUT_DMABUF_LEN;
        }
        else if (gPulserStatus == WAITING)
        {
            if (desc->qe.stqe_next == NULL)
            {
                // Tail of the DMA chain found — transition back to PASSTHROUGH
                clearAllDmaBuffers(0);
                gPulserStatus = PASSTHROUGH;
                i2sOutSetPassthrough();
            }
            else
            {
                // Not yet at the tail — keep draining
                clearDmaBuffer(desc, 0);
                gDma.fRwPos = 0;
                desc->qe.stqe_next = NULL;
            }
        }
        else
        {
            // PASSTHROUGH — fill with zeros (no output)
            clearDmaBuffer(desc, 0);
            gDma.fRwPos = 0;
        }

        portEXIT_CRITICAL(&gPulserSpinlock);
    }
}

// ---------------------------------------------------------------------------
// Public: initialize the I2S output subsystem
// ---------------------------------------------------------------------------
int i2sOutInit()
{
    if (gInitialized)
    {
        return -1; // Already initialized
    }

    // --- Allocate DMA descriptor and buffer arrays ---
    gDma.fDesc = (lldesc_t **)malloc(I2S_OUT_DMABUF_COUNT * sizeof(lldesc_t *));
    gDma.fBuffers = (uint32_t **)malloc(I2S_OUT_DMABUF_COUNT * sizeof(uint32_t *));

    for (int i = 0; i < I2S_OUT_DMABUF_COUNT; i++)
    {
        gDma.fDesc[i] = (lldesc_t *)heap_caps_malloc(sizeof(lldesc_t), MALLOC_CAP_DMA);
        gDma.fBuffers[i] = (uint32_t *)heap_caps_malloc(I2S_OUT_DMABUF_LEN, MALLOC_CAP_DMA);
        memset(gDma.fBuffers[i], 0, I2S_OUT_DMABUF_LEN);
    }

    gDma.fQueue = xQueueCreate(I2S_OUT_DMABUF_COUNT, sizeof(lldesc_t *));

    clearAllDmaBuffers(0);

    // --- Enable I2S0 peripheral clock ---
    periph_module_enable(PERIPH_I2S0_MODULE);

    // --- Configure I2S0 for standard 32-bit TX ---
    I2S0.conf.tx_reset = 1;
    I2S0.conf.tx_reset = 0;
    I2S0.conf.rx_reset = 1;
    I2S0.conf.rx_reset = 0;
    I2S0.conf.tx_msb_right = 0;
    I2S0.conf.tx_right_first = 0;
    I2S0.conf.tx_slave_mod = 0; // Master mode
    I2S0.conf.tx_short_sync = 0;
    I2S0.conf.tx_mono = 0;
    I2S0.conf.tx_msb_shift = 0;
    // NOTE: tx_bck_div_num belongs to sample_rate_conf, NOT conf — see below

    I2S0.conf2.lcd_en = 0;
    I2S0.conf2.camera_en = 0;

    // Clock: APB / (clkm_div_num * bck_div_num * 2)
    // Target: 1 bit = I2S_OUT_USEC_PER_PULSE µs  =>  ~250kHz bit clock
    I2S0.clkm_conf.clkm_div_a = 0;
    I2S0.clkm_conf.clkm_div_b = 0;
    I2S0.clkm_conf.clkm_div_num = 160; // 80MHz/160/1/2 = 250kHz WS → 32bits → 4µs/word
    I2S0.clkm_conf.clka_en = 0;        // Use APB clock (80MHz)

    I2S0.fifo_conf.tx_fifo_mod_force_en = 1;
    I2S0.fifo_conf.tx_fifo_mod = 3; // 32-bit single channel
    I2S0.fifo_conf.tx_data_num = 32;
    I2S0.fifo_conf.dscr_en = 0; // Disabled until DMA start

    I2S0.conf_chan.tx_chan_mod = 1; // Right channel only

    I2S0.sample_rate_conf.tx_bits_mod = 32;
    I2S0.sample_rate_conf.tx_bck_div_num = 1; // ✅ Correct register

    I2S0.int_ena.val = 0;
    I2S0.int_clr.val = 0xFFFFFFFF;
    I2S0.int_ena.out_eof = 1;
    I2S0.int_ena.out_total_eof = 1;

    // --- Route GPIO pins through the I2S signal matrix ---
    gpioMatrixOutSet(I2S_OUT_BCK, I2S0O_BCK_OUT_IDX);
    gpioMatrixOutSet(I2S_OUT_WS, I2S0O_WS_OUT_IDX);
    gpioMatrixOutSet(I2S_OUT_DATA, I2S0O_DATA_OUT23_IDX);

    // --- Install DMA interrupt ---
    esp_intr_alloc(ETS_I2S0_INTR_SOURCE,
                   ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL1,
                   i2sIsrHandler,
                   NULL,
                   &gIsrHandle);

    // --- Start the DMA fill task pinned to the support core ---
    xTaskCreatePinnedToCore(i2sOutTask,
                            "i2sOutTask",
                            2048,
                            NULL,
                            SUPPORT_TASK_CORE + 1,
                            NULL,
                            SUPPORT_TASK_CORE);

    gInitialized = 1;

    // Start in PASSTHROUGH mode
    i2sOutSetPassthrough();

    MessageSender::SendMessage(EMessageLevel::Info,
                               "I2S stepper output init: BCK=%d WS=%d DATA=%d",
                               I2S_OUT_BCK, I2S_OUT_WS, I2S_OUT_DATA);

    return 0;
}

// ---------------------------------------------------------------------------
// Public: write one virtual pin into the shadow register
// ---------------------------------------------------------------------------
void IRAM_ATTR i2sOutWrite(uint8_t pin, uint8_t val)
{
    uint32_t bit = (1u << pin);
    if (val)
    {
        gPortData.fetch_or(bit); // C++ atomic method
    }
    else
    {
        gPortData.fetch_and(~bit); // C++ atomic method
    }

    // In PASSTHROUGH mode, push the new state to the shift register immediately
    if (gPulserStatus == PASSTHROUGH)
    {
        i2sOutSingleData();
    }
}

// ---------------------------------------------------------------------------
// Public: read the shadow value of one virtual pin
// ---------------------------------------------------------------------------
uint8_t IRAM_ATTR i2sOutRead(uint8_t pin)
{
    return (uint8_t)(!!(gPortData.load() & (1u << pin))); // C++ atomic method
}

// ---------------------------------------------------------------------------
// Public: push current port state as a timed pulse into the DMA buffer
// Called from Stepper_I2S::Step() / Unstep() during STEPPING mode
// ---------------------------------------------------------------------------
uint32_t IRAM_ATTR i2sOutPushSample(uint32_t usec)
{
    uint32_t numSamples = usec / I2S_OUT_USEC_PER_PULSE;
    if (numSamples == 0)
    {
        numSamples = 1;
    }
    if (numSamples > (uint32_t)SAMPLE_SAFE_COUNT)
    {
        return 0; // Requested duration exceeds safe limit
    }

    uint32_t portData = gPortData.load(); // C++ atomic method
    for (uint32_t i = 0; i < numSamples; i++)
    {
        gDma.fCurrent[gDma.fRwPos++] = portData;
    }
    return numSamples;
}

// ---------------------------------------------------------------------------
// Public: switch to PASSTHROUGH (static) mode
// ---------------------------------------------------------------------------
int i2sOutSetPassthrough()
{
    portENTER_CRITICAL(&gPulserSpinlock);
    gPulserStatus = PASSTHROUGH;
    portEXIT_CRITICAL(&gPulserSpinlock);

    i2sOutStop();

    // Apply current pin state directly via the single-data register
    I2S0.conf_single_data = gPortData.load(); // C++ atomic method
    I2S0.conf.tx_start = 1;                   // Keep clock running for shift register latch

    return 0;
}

// ---------------------------------------------------------------------------
// Public: switch to STEPPING (DMA streaming) mode
// ---------------------------------------------------------------------------
int i2sOutSetStepping()
{
    portENTER_CRITICAL(&gPulserSpinlock);
    gPulserStatus = STEPPING;
    portEXIT_CRITICAL(&gPulserSpinlock);

    clearAllDmaBuffers(gPortData.load()); // C++ atomic method
    i2sOutStart();
    return 0;
}

// ---------------------------------------------------------------------------
// Public: query the current pulser state
// ---------------------------------------------------------------------------
I2sOutPulserStatus i2sOutGetPulserStatus()
{
    portENTER_CRITICAL(&gPulserSpinlock);
    I2sOutPulserStatus status = gPulserStatus;
    portEXIT_CRITICAL(&gPulserSpinlock);
    return status;
}

// ---------------------------------------------------------------------------
// Public: mode-aware delay
// ---------------------------------------------------------------------------
void i2sOutDelay()
{
    portENTER_CRITICAL(&gPulserSpinlock);
    I2sOutPulserStatus status = gPulserStatus;
    portEXIT_CRITICAL(&gPulserSpinlock);

    if (status == PASSTHROUGH)
    {
        ets_delay_us(I2S_OUT_USEC_PER_PULSE * 2);
    }
    else
    {
        vTaskDelay(pdMS_TO_TICKS(I2S_OUT_DELAY_MS));
    }
}

// ---------------------------------------------------------------------------
// Public: full reset — stop DMA, clear all buffers, return to PASSTHROUGH
// ---------------------------------------------------------------------------
int i2sOutReset()
{
    portENTER_CRITICAL(&gPulserSpinlock);
    gPulserStatus = PASSTHROUGH;
    portEXIT_CRITICAL(&gPulserSpinlock);

    i2sOutStop();
    clearAllDmaBuffers(0);
    gPortData.store(0u); 
    I2S0.conf_single_data = 0;

    return 0;
}
#endif