/*
    Spindle_PWM.cpp

    Copyright (c) 2026 Nikolaos Siatras
    Twitter: nsiatras
    Github: https://github.com/nsiatras
    Website: https://www.sourcerabbit.com

    Rabbit GRBLis free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rabbit GRBL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Spindle_PWM.h"
#include "soc/ledc_struct.h"
#include "../System.h"

namespace Spindles
{
    void PWM::Initialize()
    {
        Spindle::Initialize(); // call base (pins, rpm range, delays)

        if (fOutputPin == UNDEFINED_PIN)
        {
            // Base initialization failed (no output pin configured).
            grbl_msg_sendf(MsgLevel::Info, "Warning: PWM Spindle output pin not defined");
            return;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize PWM Output ////////////////////////////////////////////////////////////////////////////////////////////////////////////
        const uint8_t channelNumber = 0; // Channel 0 is reserved for spindle use
        const uint32_t pwmFrequency = settings_spindle_pwm_freq->get();
        const uint8_t pwmPrecision = system_calculate_pwm_precision(pwmFrequency);
        const uint32_t pwmPeriod = (1UL << pwmPrecision);
        const uint32_t pwmOffValue = (uint32_t)(pwmPeriod * settings_spindle_pwm_off_value->get() / 100.0);
        const uint32_t pwmMinValue = (uint32_t)(pwmPeriod * settings_spindle_pwm_min_value->get() / 100.0);
        const uint32_t pwmMaxValue = (uint32_t)(pwmPeriod * settings_spindle_pwm_max_value->get() / 100.0);
        const bool invertPWM = settings_spindle_output_invert->get();
        InitializePWMOutput(channelNumber, pwmFrequency, pwmPrecision, pwmPeriod, pwmOffValue, pwmMinValue, pwmMaxValue, invertPWM);
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Setup optional pins
        if (fEnablePin != UNDEFINED_PIN)
        {
            pinMode(fEnablePin, OUTPUT);
            setEnablePinValue(false);
        }

        if (fDirectionPin != UNDEFINED_PIN)
        {
            pinMode(fDirectionPin, OUTPUT);
        }

        // Safe initial state
        setRPM(0);
        Stop();

        // Display config message only for pure PWM spindle (not for derived types like BESC or Laser)
        if (static_cast<ESpindleType>(settings_spindle_type->get()) == ESpindleType::PWM)
        {
            grbl_msg_sendf(MsgLevel::Info,
                           "PWM spindle on Pin:%d Off:%.1f%% Min:%.1f%% Max:%.1f%% Freq:%dHz Res:%dbits",
                           fOutputPin,
                           settings_spindle_pwm_off_value->get(),
                           settings_spindle_pwm_min_value->get(),
                           settings_spindle_pwm_max_value->get(),
                           fPWMFrequency,
                           fPWMPrecision);
        }
    }

    /**
     *  Initializes and configures the LEDC PWM output for the spindle.
        Stores all PWM parameters (channel, frequency, resolution/precision, period, OFF/MIN/MAX duty, inversion),
        resets the runtime spindle/PWM state, sets up the LEDC channel, attaches it to the configured output pin,
        and forces the initial output to the configured OFF duty for a safe startup state.
     */
    void PWM::InitializePWMOutput(uint8_t channelNumber, uint32_t frequency, uint8_t precision, uint32_t period, uint32_t offValue,
                                  uint32_t minValue, uint32_t maxValue, bool invert)
    {
        // Store configuration
        fPWMChannelNumber = channelNumber;
        fPWMFrequency = frequency;
        fPWMPrecision = precision;
        fPWMPeriod = period;

        fPWMOffValue = offValue;
        fPWMMinValue = minValue;
        fPWMMaxValue = maxValue;

        fInvertPWM = invert;

        // Setup the PWM channel
        ledcSetup(fPWMChannelNumber, (double)fPWMFrequency, fPWMPrecision);

        // Attach the PWM to the output pin
        pinMode(fOutputPin, OUTPUT);
        ledcAttachPin(fOutputPin, fPWMChannelNumber);
    }

    void PWM::Stop()
    {
        // inverts are delt with in methods
        setEnablePinValue(false);
        setPWMOutput(fPWMOffValue);
    }

    uint32_t PWM::setRPM(uint32_t rpm)
    {
        // Clamp requested RPM to the configured range.
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;
        rpm = (rpm > 0 && rpm < fMinRPM) ? fMinRPM : rpm;

        // Apply override (percent) while RPM > 0.
        rpm = (rpm > 0) ? (uint32_t)(((uint64_t)rpm * sys.spindle_speed_ovr) / 100ULL) : 0;

        // Clamp again after overrides are applied.
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;
        rpm = (rpm > 0 && rpm < fMinRPM) ? fMinRPM : rpm;

        // In case the effective RPM did not change,
        // or the fOutputPin is Undefined
        // do nothing.
        if (rpm == sys.spindle_speed || fOutputPin == UNDEFINED_PIN)
        {
            return rpm;
        }

        sys.spindle_speed = rpm;

        // Notice: RPM==0 turns PWM off.
        // For RPM > 0, map duty using the full 0..MaxRPM range, but never allow duty < MinPWM.
        uint32_t pwmValue = fPWMOffValue;

        if (rpm > 0)
        {
            pwmValue = map_uint32_t(rpm, 0, fMaxRPM, fPWMMinValue, fPWMMaxValue);

            // Ensure non-zero RPM never produces less than the configured minimum PWM.
            if (pwmValue < fPWMMinValue)
            {
                pwmValue = fPWMMinValue;
            }
        }

        this->setEnablePinValue(gc_state.modal.spindle != SpindleState::Disable);
        this->setPWMOutput(pwmValue);

        return rpm;
    }

    void PWM::setState(SpindleState state, uint32_t rpm)
    {
        if (sys.abort)
        {
            return; // Block during abort.
        }

        // Detect a CW <-> CCW direction change while the spindle remains enabled.
        const bool wasEnabled = (fCurrentState != SpindleState::Disable);
        const bool willBeEnabled = (state != SpindleState::Disable);
        const bool directionChange = this->isReversable() && wasEnabled && willBeEnabled && (fCurrentState != state);

        if (state == SpindleState::Disable)
        {
            // Halt or set spindle direction and rpm.
            sys.spindle_speed = 0;
            Stop();

            if (fCurrentState != state)
            {
                // grbl_msg_sendf(MsgLevel::Info, "Spin down delay");
                delay(this->getSpinDownDelay());
            }
        }
        else
        {
            if (directionChange)
            {
                // Safe reverse: stop first, wait for spin-down,
                // then change direction and restart.
                sys.spindle_speed = 0;
                Stop();

                // Allow spindle to spin down !
                delay(this->getSpinUpDelay());
            }

            // Apply direction first, then RPM/PWM, then enable (implementation-specific).
            setDirectionPinValue(state == SpindleState::Cw);
            setRPM(rpm);
            setEnablePinValue(true);

            if (fCurrentState != state)
            {
                // Allow spindle to spin up.
                delay(this->getSpinUpDelay());
            }
        }

        fCurrentState = state;
        sys.report_ovr_counter = 0; // Set to report change immediately
    }

    SpindleState PWM::getState()
    {
        if (fCurrentPWMDuty == 0 || fOutputPin == UNDEFINED_PIN)
        {
            return SpindleState::Disable;
        }

        if (fDirectionPin != UNDEFINED_PIN)
        {
            return digitalRead(fDirectionPin) ? SpindleState::Cw : SpindleState::Ccw;
        }

        return SpindleState::Cw;
    }

    void PWM::setPWMOutput(uint32_t duty)
    {
        if (fOutputPin == UNDEFINED_PIN)
        {
            return;
        }

        // to prevent excessive calls to ledcWrite, make sure duty hass changed
        if (duty == fCurrentPWMDuty)
        {
            return;
        }

        fCurrentPWMDuty = duty;

        if (fInvertPWM)
        {
            duty = (1 << fPWMPrecision) - duty;
        }

        // Map Arduino LEDC channel number to LEDC group/channel indices.
        const uint8_t group = (fPWMChannelNumber >= 8) ? 1 : 0;
        const uint8_t ch = (uint8_t)(fPWMChannelNumber & 0x07);

        // Write duty in fixed-point format (duty << 4).
        LEDC.channel_group[group].channel[ch].duty.duty = duty << 4;

        const bool on = (duty != 0);
        LEDC.channel_group[group].channel[ch].conf0.sig_out_en = on;
        LEDC.channel_group[group].channel[ch].conf1.duty_start = 1; // trigger duty update
        LEDC.channel_group[group].channel[ch].conf0.clk_en = on;
    }

    void PWM::setEnablePinValue(bool active)
    {
        if (fEnablePin == UNDEFINED_PIN)
        {
            return;
        }

        if (sys.spindle_speed == 0)
        {
            active = false;
        }

        if (settings_spindle_enable_invert->get())
        {
            active = !active;
        }

        digitalWrite(fEnablePin, active);
    }

    void PWM::setDirectionPinValue(bool Clockwise)
    {
        // Check if direction pin is configured before attempting to write
        if (fDirectionPin == UNDEFINED_PIN)
        {
            return;
        }

        digitalWrite(fDirectionPin, Clockwise);
    }

    void PWM::Dispose()
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
