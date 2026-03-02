/*
    I2SOut.h

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

#pragma once

#include "Grbl.h"
#include <stdint.h>
#include <driver/gpio.h>

// ---------------------------------------------------------------------------
// Timing constants
// Each DMA "sample" is one 32-bit word clocked out at I2S rate.
// I2S_OUT_USEC_PER_PULSE defines how many microseconds one sample lasts.
// ---------------------------------------------------------------------------
constexpr int I2S_OUT_USEC_PER_PULSE = 4; // microseconds per DMA sample (= minimum step pulse width)
constexpr int I2S_OUT_DMABUF_COUNT = 5;   // number of DMA ring buffers
constexpr int I2S_OUT_DMABUF_LEN = 2000;  // bytes per DMA buffer

// Derived timing: total DMA latency in milliseconds
constexpr int I2S_OUT_DELAY_DMABUF_MS = (I2S_OUT_DMABUF_LEN / 4 * I2S_OUT_USEC_PER_PULSE / 1000);
constexpr int I2S_OUT_DELAY_MS = (I2S_OUT_DELAY_DMABUF_MS * (I2S_OUT_DMABUF_COUNT + 1));

// ---------------------------------------------------------------------------
// I2S output pin configuration (override in machine header if needed)
// ---------------------------------------------------------------------------
#ifndef I2S_OUT_BCK
#define I2S_OUT_BCK GPIO_NUM_22 // Bit clock  -> 74HC595 SRCLK
#endif
#ifndef I2S_OUT_WS
#define I2S_OUT_WS GPIO_NUM_17 // Word select -> 74HC595 RCLK (latch)
#endif
#ifndef I2S_OUT_DATA
#define I2S_OUT_DATA GPIO_NUM_21 // Serial data -> 74HC595 SER
#endif

// ---------------------------------------------------------------------------
// Pulser state machine
//   PASSTHROUGH : static mode — i2sOutWrite() updates the shift register immediately
//   STEPPING    : streaming mode — DMA feeds timed step pulses
//   WAITING     : transitional — waiting for DMA pipeline to drain
// ---------------------------------------------------------------------------
typedef enum
{
    PASSTHROUGH = 0,
    STEPPING,
    WAITING
} I2sOutPulserStatus;

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

// Initialize the I2S output subsystem (call once at startup)
int i2sOutInit();

// Write a single virtual pin value (pin = bit index 0..31, val = 0 or 1)
void i2sOutWrite(uint8_t pin, uint8_t val);

// Read the current shadow value of a virtual pin
uint8_t i2sOutRead(uint8_t pin);

// Push the current port state as a pulse of 'usec' microseconds into the DMA buffer.
// Returns the number of DMA samples consumed.
uint32_t i2sOutPushSample(uint32_t usec);

// Switch to PASSTHROUGH (static) mode
int i2sOutSetPassthrough();

// Switch to STEPPING (DMA streaming) mode
int i2sOutSetStepping();

// Query the current pulser state
I2sOutPulserStatus i2sOutGetPulserStatus();

// Mode-aware delay: short ets_delay in PASSTHROUGH, vTaskDelay in STEPPING
void i2sOutDelay();

// Full reset: stops DMA, clears buffers, returns to PASSTHROUGH
int i2sOutReset();
