/*
  PWMSpindle.cpp

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
  Website: https://www.sourcerabbit.com

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "PWMSpindle.h"
#include "soc/ledc_struct.h"

namespace Spindles
{
    void PWM::Initialize()
    {
        Spindle::Initialize(); // call base

        fInvertPWM = settings_spindle_output_invert->get();
        fPWMFrequency = settings_spindle_pwm_freq->get();
        fPWMPrecision = CalculatePWMPrecision(fPWMFrequency); // detewrmine the best precision
        fPWMPeriod = (1 << fPWMPrecision);

        if (settings_spindle_pwm_min_value->get() > settings_spindle_pwm_max_value->get())
        {
            grbl_msg_sendf(MsgLevel::Info, "Warning: Spindle min pwm is greater than max. Check $35 and $36");
        }

        // Pre-calculate some PWM count values
        fPWMOffValue = (fPWMPeriod * settings_spindle_pwm_off_value->get() / 100.0);
        fPWMMinValue = (fPWMPeriod * settings_spindle_pwm_min_value->get() / 100.0);
        fPWMMaxValue = (fPWMPeriod * settings_spindle_pwm_max_value->get() / 100.0);

        fMinRPM = settings_spindle_rpm_min->get();
        fMaxRPM = settings_spindle_rpm_max->get();

        fPWMChannelNumber = 0; // Channel 0 is reserved for spindle use

        fCurrentState = SpindleState::Disable;
        fCurrentPWMDuty = 0;

        ledcSetup(fPWMChannelNumber, (double)fPWMFrequency, fPWMPrecision); // setup the channel
        ledcAttachPin(fOutputPin, fPWMChannelNumber);                       // attach the PWM to the pin

        if (fEnablePin != UNDEFINED_PIN)
        {
            pinMode(fEnablePin, OUTPUT);
        }

        if (fDirectionPin != UNDEFINED_PIN)
        {
            pinMode(fDirectionPin, OUTPUT);
        }
    }

    void PWM::Stop()
    {
        // inverts are delt with in methods
        setEnablePinValue(false);
        setPWMOutput(fPWMOffValue);
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

    uint32_t PWM::setRPM(uint32_t rpm)
    {
        // Initial RPM range clamp
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;
        rpm = (rpm > 0 && rpm < fMinRPM) ? fMinRPM : rpm;

        // Apply override (percent) while RPM > 0
        rpm = (rpm > 0) ? (uint32_t)(((uint64_t)rpm * sys.spindle_speed_ovr) / 100ULL) : 0;

        // RPM range clamp after overrides applied !
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;
        rpm = (rpm > 0 && rpm < fMinRPM) ? fMinRPM : rpm;

        if (fOutputPin == UNDEFINED_PIN)
        {
            return rpm;
        }

        sys.spindle_speed = rpm;

        // Notice: RPM==0 turns PWM off !
        const uint32_t pwmValue = (rpm == 0) ? this->fPWMOffValue : map_uint32_t(rpm, fMinRPM, fMaxRPM, fPWMMinValue, fPWMMaxValue);

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
                delay(fSpinDownDelay);
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
                delay(fSpinDownDelay);
            }

            // Apply direction first, then RPM/PWM, then enable (implementation-specific).
            setDirectionPinValue(state == SpindleState::Cw);
            setRPM(rpm);
            setEnablePinValue(true);

            if (fCurrentState != state)
            {
                // Allow spindle to spin up.
                delay(fSpinUpDelay);
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

        // ledcWrite(_pwm_chan_num, duty);

        // This was ledcWrite, but this is called from an ISR
        // and ledcWrite uses RTOS features not compatible with ISRs
        LEDC.channel_group[0].channel[0].duty.duty = duty << 4;
        bool on = !!duty;
        LEDC.channel_group[0].channel[0].conf0.sig_out_en = on;
        LEDC.channel_group[0].channel[0].conf1.duty_start = on;
        LEDC.channel_group[0].channel[0].conf0.clk_en = on;
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
        digitalWrite(fDirectionPin, Clockwise);
    }

    /*
        Calculate the maximum PWM precision (in bits) for a given frequency.

        The PWM period (in timer counts) is:
            period = 80,000,000 / freq

        We select the highest precision such that:
            (1 << precision) < period

        Notes:
        - Precision is capped at 16 bits.
        - freq must be non-zero.
    */
    uint8_t PWM::CalculatePWMPrecision(uint32_t freq)
    {
        // Protect against invalid frequency.
        if (freq == 0)
        {
            return 0;
        }

        const uint32_t period = 80000000UL / freq; // APB clock / PWM frequency
        uint8_t precision = 0;

        // Find the highest precision (bits) such that (1 << precision) < period, capped at 16 bits.
        while (precision < 16 && ((1UL << precision) < period))
        {
            ++precision;
        }

        // precision is now 1 past the valid value (or hit the cap), so step back if possible.
        return (precision == 0) ? 0 : (precision - 1);
    }

}
