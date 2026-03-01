/*
  System.cpp - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

    2018 -	Bart Dring This file was modified for use on the ESP32
                    CPU. Do not use this with Grbl for atMega328P

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
#include "Config.h"
#include "MachineCommon.h"

// Declare system global variable structure
system_t sys;
int32_t sys_position[MAX_N_AXIS];                      // Real-time machine (aka home) position vector in steps.
int32_t sys_probe_position[MAX_N_AXIS];                // Last probe position in machine coordinates and steps.
volatile ExecState sys_rt_exec_state;                  // Global realtime executor bitflag variable for state management. See EXEC bitmasks.
volatile EAlarm sys_rt_exec_alarm;                     // Global realtime executor bitflag variable for setting various alarms.
volatile ExecAccessory sys_rt_exec_accessory_override; // Global realtime executor bitflag variable for spindle/coolant overrides.
volatile bool cycle_stop;                              // For state transitions, instead of bitflag

volatile Percent sys_rt_f_override; // Global realtime executor feedrate override percentage
volatile Percent sys_rt_r_override; // Global realtime executor rapid override percentage
volatile Percent sys_rt_s_override; // Global realtime executor spindle override percentage

xQueueHandle control_sw_queue; // used by control switch debouncing
bool debouncing = false;       // debouncing in process

void system_ini()
{ // Renamed from system_init() due to conflict with esp32 files
  // setup control inputs

#ifdef CONTROL_SAFETY_DOOR_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Door switch on pin %s", pinName(CONTROL_SAFETY_DOOR_PIN).c_str());
    pinMode(CONTROL_SAFETY_DOOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_SAFETY_DOOR_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef CONTROL_RESET_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Reset switch on pin %s", pinName(CONTROL_RESET_PIN).c_str());
    pinMode(CONTROL_RESET_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_RESET_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef CONTROL_FEED_HOLD_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Hold switch on pin %s", pinName(CONTROL_FEED_HOLD_PIN).c_str());
    pinMode(CONTROL_FEED_HOLD_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_FEED_HOLD_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef CONTROL_CYCLE_START_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Start switch on pin %s", pinName(CONTROL_CYCLE_START_PIN).c_str());
    pinMode(CONTROL_CYCLE_START_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_CYCLE_START_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_0_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Macro Pin 0 %s", pinName(MACRO_BUTTON_0_PIN).c_str());
    pinMode(MACRO_BUTTON_0_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_0_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_1_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Macro Pin 1 %s", pinName(MACRO_BUTTON_1_PIN).c_str());
    pinMode(MACRO_BUTTON_1_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_1_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_2_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Macro Pin 2 %s", pinName(MACRO_BUTTON_2_PIN).c_str());
    pinMode(MACRO_BUTTON_2_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_2_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_3_PIN
    MessageSender::SendMessage(EMessageLevel::Info, "Macro Pin 3 %s", pinName(MACRO_BUTTON_3_PIN).c_str());
    pinMode(MACRO_BUTTON_3_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_3_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef ENABLE_CONTROL_SW_DEBOUNCE
    // setup task used for debouncing
    control_sw_queue = xQueueCreate(10, sizeof(int));
    xTaskCreate(controlCheckTask,
                "controlCheckTask",
                3096,
                NULL,
                5, // priority
                NULL);
#endif

    // customize pin definition if needed
#if (GRBL_SPI_SS != -1) || (GRBL_SPI_MISO != -1) || (GRBL_SPI_MOSI != -1) || (GRBL_SPI_SCK != -1)
    SPI.begin(GRBL_SPI_SCK, GRBL_SPI_MISO, GRBL_SPI_MOSI, GRBL_SPI_SS);
#endif
}

#ifdef ENABLE_CONTROL_SW_DEBOUNCE
// this is the debounce task
void controlCheckTask(void *pvParameters)
{
    while (true)
    {
        int evt;
        xQueueReceive(control_sw_queue, &evt, portMAX_DELAY); // block until receive queue
        vTaskDelay(CONTROL_SW_DEBOUNCE_PERIOD);               // delay a while
        ControlPins pins = system_control_get_state();
        if (pins.value)
        {
            system_exec_control_pin(pins);
        }
        debouncing = false;
    }
}
#endif

void IRAM_ATTR isr_control_inputs()
{
#ifdef ENABLE_CONTROL_SW_DEBOUNCE
    // we will start a task that will recheck the switches after a small delay
    int evt;
    if (!debouncing)
    { // prevent resending until debounce is done
        debouncing = true;
        xQueueSendFromISR(control_sw_queue, &evt, NULL);
    }
#else
    ControlPins pins = system_control_get_state();
    system_exec_control_pin(pins);
#endif
}

// Returns if safety door is ajar(T) or closed(F), based on pin state.
uint8_t system_check_safety_door_ajar()
{
#ifdef ENABLE_SAFETY_DOOR_INPUT_PIN
    return system_control_get_state().bit.safetyDoor;
#else
    return false; // Input pin not enabled, so just return that it's closed.
#endif
}

void system_flag_wco_change()
{
#ifdef FORCE_BUFFER_SYNC_DURING_WCO_CHANGE
    protocol_buffer_synchronize();
#endif
    sys.report_wco_counter = 0;
}

// Returns machine position of axis 'idx'. Must be sent a 'step' array.
// NOTE: If motor steps and machine position are not in the same coordinate frame, this function
//   serves as a central place to compute the transformation.
float system_convert_axis_steps_to_mpos(int32_t *steps, uint8_t idx)
{
    float pos;
    float steps_per_mm = axis_settings[idx]->steps_per_mm->get();
    pos = (steps[idx] / steps_per_mm);
    return pos;
}

void system_convert_array_steps_to_mpos(float *position, int32_t *steps)
{
    uint8_t idx;
    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++)
    {
        position[idx] = system_convert_axis_steps_to_mpos(steps, idx);
    }
    return;
}

// Returns control pin state as a uint8 bitfield. Each bit indicates the input pin state, where
// triggered is 1 and not triggered is 0. Invert mask is applied. Bitfield organization is
// defined by the ControlPin in System.h.
ControlPins system_control_get_state()
{
    ControlPins defined_pins;
    defined_pins.value = 0;

    ControlPins pin_states;
    pin_states.value = 0;

#ifdef CONTROL_SAFETY_DOOR_PIN
    defined_pins.bit.safetyDoor = true;
    if (digitalRead(CONTROL_SAFETY_DOOR_PIN))
    {
        pin_states.bit.safetyDoor = true;
    }
#endif
#ifdef CONTROL_RESET_PIN
    defined_pins.bit.reset = true;
    if (digitalRead(CONTROL_RESET_PIN))
    {
        pin_states.bit.reset = true;
    }
#endif
#ifdef CONTROL_FEED_HOLD_PIN
    defined_pins.bit.feedHold = true;
    if (digitalRead(CONTROL_FEED_HOLD_PIN))
    {
        pin_states.bit.feedHold = true;
    }
#endif
#ifdef CONTROL_CYCLE_START_PIN
    defined_pins.bit.cycleStart = true;
    if (digitalRead(CONTROL_CYCLE_START_PIN))
    {
        pin_states.bit.cycleStart = true;
    }
#endif
#ifdef MACRO_BUTTON_0_PIN
    defined_pins.bit.macro0 = true;
    if (digitalRead(MACRO_BUTTON_0_PIN))
    {
        pin_states.bit.macro0 = true;
    }
#endif
#ifdef MACRO_BUTTON_1_PIN
    defined_pins.bit.macro1 = true;
    if (digitalRead(MACRO_BUTTON_1_PIN))
    {
        pin_states.bit.macro1 = true;
    }
#endif
#ifdef MACRO_BUTTON_2_PIN
    defined_pins.bit.macro2 = true;
    if (digitalRead(MACRO_BUTTON_2_PIN))
    {
        pin_states.bit.macro2 = true;
    }
#endif
#ifdef MACRO_BUTTON_3_PIN
    defined_pins.bit.macro3 = true;
    if (digitalRead(MACRO_BUTTON_3_PIN))
    {
        pin_states.bit.macro3 = true;
    }
#endif
#ifdef INVERT_CONTROL_PIN_MASK
    pin_states.value ^= (INVERT_CONTROL_PIN_MASK & defined_pins.value);
#endif
    return pin_states;
}

// execute the function of the control pin
void system_exec_control_pin(ControlPins pins)
{
    if (pins.bit.reset)
    {
        MessageSender::SendMessage(EMessageLevel::Info, "Reset via control pin");
        mc_reset();
    }
    else if (pins.bit.cycleStart)
    {
        sys_rt_exec_state.bit.cycleStart = true;
    }
    else if (pins.bit.feedHold)
    {
        sys_rt_exec_state.bit.feedHold = true;
    }
    else if (pins.bit.safetyDoor)
    {
        sys_rt_exec_state.bit.safetyDoor = true;
    }
    else if (pins.bit.macro0)
    {
        user_defined_macro(0); // function must be implemented by user
    }
    else if (pins.bit.macro1)
    {
        user_defined_macro(1); // function must be implemented by user
    }
    else if (pins.bit.macro2)
    {
        user_defined_macro(2); // function must be implemented by user
    }
    else if (pins.bit.macro3)
    {
        user_defined_macro(3); // function must be implemented by user
    }
}

/*
    This returns an unused pwm channel.
    The 8 channels share 4 timers, so pairs 0,1 & 2,3 , etc
    have to be the same frequency. The spindle always uses channel 0
    so we start counting from 2.

    There are still possible issues if requested channels use different frequencies
    TODO: Make this more robust.
*/
int8_t sys_get_next_PWM_chan_num()
{
    static uint8_t next_PWM_chan_num = 2; // start at 2 to avoid spindle
    if (next_PWM_chan_num < 8)
    {
        // 7 is the max PWM channel number
        return next_PWM_chan_num++;
    }
    else
    {
        MessageSender::SendMessage(EMessageLevel::Error, "Error: out of PWM channels");
        return -1;
    }
}

/*
    Compute the maximum usable LEDC PWM resolution (in bits) for a given PWM frequency.

    On ESP32, the LEDC timer is driven by the APB clock (typically 80 MHz).
    For a requested PWM frequency, the number of timer ticks per PWM period is:

        period_ticks = ESP32_TIMERS_CLOCK / freq

    The PWM resolution (bits) determines how many discrete duty steps are available:
        duty_steps = 2^bits

    We select the highest resolution (1..16 bits) such that the duty range fits within one
    PWM period:

        2^bits <= period_ticks

    This yields the best possible duty granularity for the given frequency without
    exceeding the timer's capacity.

    Notes:
    - LEDC supports up to 16-bit resolution.
    - freq must be non-zero; if freq == 0 we return 1 (minimum valid value for ledcSetup()).
*/
uint8_t system_calculate_pwm_precision(uint32_t freq)
{
    // Protect against invalid frequency.
    if (freq == 0)
    {
        return 1; // Minimum valid resolution for ledcSetup().
    }

    // For a classic ESP32, the LEDC timer clock is typically the 80 MHz APB clock.
    // period_ticks = timerClockHz / freq
    const uint32_t period = ESP32_TIMERS_CLOCK / freq; // ESP32 APB clock ticks per PWM period

    uint8_t precision = 0;

    // Find the highest precision such that (1 << precision) < period, capped at 16 bits.
    while (precision < 16 && ((1UL << precision) < period))
    {
        ++precision;
    }

    // Ensure we always return a valid resolution (1..16).
    if (precision == 0)
    {
        return 1;
    }

    const uint8_t bits = precision - 1;
    return (bits == 0) ? 1 : bits;
}

void __attribute__((weak)) user_defined_macro(uint8_t index)
{
    // must be in Idle
    if (sys.state != State::Idle)
    {
        MessageSender::SendMessage(EMessageLevel::Info, "Macro button only permitted in idle");
        return;
    }

    String user_macro;
    char line[255];
    switch (index)
    {
    case 0:
        user_macro = user_macro0->get();
        break;
    case 1:
        user_macro = user_macro1->get();
        break;
    case 2:
        user_macro = user_macro2->get();
        break;
    case 3:
        user_macro = user_macro3->get();
        break;
    default:
        return;
    }

    if (user_macro == "")
    {
        MessageSender::SendMessage(EMessageLevel::Info, "Macro User/Macro%d empty", index);
        return;
    }

    user_macro.replace('&', '\n');
    user_macro.toCharArray(line, 255, 0);
    strcat(line, "\r");
    ConnectionManager::Active().Push(line);
}
