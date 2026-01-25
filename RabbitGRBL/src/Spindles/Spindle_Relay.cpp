/*
    Spindle_Relay.cpp

    Copyright (c) 2026 Nikolaos Siatras
    Twitter: nsiatras
    Github: https://github.com/nsiatras
    Website: https://www.sourcerabbit.com

    Rabbit GRBL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rabbit GRBL  is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Spindle_Relay.h"

namespace Spindles
{
    void Relay::Initialize()
    {
        // Resolve pins, rpm limits and delays using the base spindle initialization.
        Spindle::Initialize();

        if (fOutputPin == UNDEFINED_PIN)
        {
            // Base initialization failed (no output pin configured).
            return;
        }

        // Prepare GPIOs.
        pinMode(fOutputPin, OUTPUT);

        if (fEnablePin != UNDEFINED_PIN)
        {
            pinMode(fEnablePin, OUTPUT);
        }

        if (fDirectionPin != UNDEFINED_PIN)
        {
            pinMode(fDirectionPin, OUTPUT);
        }

        // Ensure everything is off at start.
        fCurrentState = SpindleState::Disable;
        fOutputOn = false;

        setDirectionPinValue(true); // Default direction (CW)
        setEnablePinValue(false);
        setRelayOutput(false);
    }

    uint32_t Relay::setRPM(uint32_t rpm)
    {
        // Clamp to max for safety.
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;

        // Apply override (percent) while RPM > 0.
        rpm = (rpm > 0) ? (uint32_t)(((uint64_t)rpm * sys.spindle_speed_ovr) / 100ULL) : 0;

        sys.spindle_speed = rpm;

        // Relay is simply ON when rpm > 0.
        const bool on = (rpm > 0);

        // Enable pin follows modal state, but will be forced OFF if rpm == 0 inside setEnablePinValue().
        setEnablePinValue(gc_state.modal.spindle != SpindleState::Disable);

        setRelayOutput(on);

        return rpm;
    }

    void Relay::setState(SpindleState state, uint32_t rpm)
    {
        if (sys.abort)
        {
            return; // Block during abort.
        }

        // Detect a CW <-> CCW direction change while spindle remains enabled.
        const bool wasEnabled = (fCurrentState != SpindleState::Disable);
        const bool willBeEnabled = (state != SpindleState::Disable);
        const bool directionChange = isReversable() && wasEnabled && willBeEnabled && (fCurrentState != state);

        if (state == SpindleState::Disable)
        {
            sys.spindle_speed = 0;
            Stop();

            if (fCurrentState != state)
            {
                delay(fSpinDownDelayMs);
            }
        }
        else
        {
            if (directionChange)
            {
                // Safe reverse: stop, wait, then change direction and restart.
                sys.spindle_speed = 0;
                Stop();
                delay(fSpinDownDelayMs);
            }

            // Apply direction first, then RPM, then enable.
            setDirectionPinValue(state == SpindleState::Cw);
            setRPM(rpm);
            setEnablePinValue(true);

            if (fCurrentState != state)
            {
                delay(fSpinUpDelayMs);
            }
        }

        fCurrentState = state;
        sys.report_ovr_counter = 0; // Report change immediately
    }

    SpindleState Relay::getState()
    {
        if (fOutputPin == UNDEFINED_PIN || !fOutputOn)
        {
            return SpindleState::Disable;
        }

        if (fDirectionPin != UNDEFINED_PIN)
        {
            return digitalRead(fDirectionPin) ? SpindleState::Cw : SpindleState::Ccw;
        }

        return SpindleState::Cw;
    }

    void Relay::Stop()
    {
        // Ensure spindle is really off.
        setEnablePinValue(false);
        setRelayOutput(false);
    }

    bool Relay::isReversable()
    {
        // Reversing is possible only when a valid direction GPIO pin is configured.
        return (fDirectionPin != UNDEFINED_PIN);
    }

    void Relay::setRelayOutput(bool on)
    {
        fOutputOn = on;
        digitalWrite(fOutputPin, on);
    }

    void Relay::setEnablePinValue(bool enabled)
    {
        if (fEnablePin == UNDEFINED_PIN)
        {
            return;
        }

        // If spindle speed is zero, force disable.
        if (sys.spindle_speed == 0)
        {
            enabled = false;
        }

        // Apply optional enable polarity inversion from settings.
        if (settings_spindle_enable_invert->get())
        {
            enabled = !enabled;
        }

        digitalWrite(fEnablePin, enabled);
    }

    void Relay::setDirectionPinValue(bool clockwise)
    {
        if (fDirectionPin == UNDEFINED_PIN)
        {
            return;
        }

        digitalWrite(fDirectionPin, clockwise);
    }

    void Relay::Dispose()
    {
        Stop();

#ifdef SPINDLE_OUTPUT_PIN
        gpio_reset_pin(SPINDLE_OUTPUT_PIN);
        pinMode(SPINDLE_OUTPUT_PIN, INPUT);
#endif
#ifdef SPINDLE_ENABLE_PIN
        gpio_reset_pin(SPINDLE_ENABLE_PIN);
        pinMode(SPINDLE_ENABLE_PIN, INPUT);
#endif
#ifdef SPINDLE_DIR_PIN
        gpio_reset_pin(SPINDLE_DIR_PIN);
        pinMode(SPINDLE_DIR_PIN, INPUT);
#endif
    }
}