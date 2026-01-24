/*
    PWMSpindle.cpp

    This is a full featured TTL PWM spindle This does not include speed/power
    compensation. Use the Laser class for that.

    Part of Grbl_ESP32
    2020 -	Bart Dring

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

    Modifications for RabbitGRBL
    2026-01-23 - Nikos Siatras
    - Optimized PWM::set_rpm() (override math and RPM limiting logic)

*/
#include "PWMSpindle.h"
#include "soc/ledc_struct.h"

// ======================= PWM ==============================
/*
    This gets called at startup or whenever a spindle setting changes
    If the spindle is running it will stop and need to be restarted with M3Snnnn
*/

// #include "grbl.h"

namespace Spindles
{
    void PWM::Initialize()
    {
        get_pins_and_settings();

        if (fOutputPin == UNDEFINED_PIN)
        {
            return; // We cannot continue without the output pin
        }

        fCurrentState = SpindleState::Disable;
        fCurrentPWMDuty = 0;
        fUseDelays = true;

        ledcSetup(fPWMChannelNumber, (double)fPWMFrequency, fPWMPrecision); // setup the channel
        ledcAttachPin(fOutputPin, fPWMChannelNumber);                       // attach the PWM to the pin
        pinMode(fEnablePin, OUTPUT);
        pinMode(fDirectionPin, OUTPUT);
    }

    // Get the GPIO from the machine definition
    void PWM::get_pins_and_settings()
    {
        // setup all the pins

#ifdef SPINDLE_OUTPUT_PIN
        fOutputPin = SPINDLE_OUTPUT_PIN;
#else
        _output_pin = UNDEFINED_PIN;
#endif

        fInvertPWM = settings_spindle_output_invert->get();

#ifdef SPINDLE_ENABLE_PIN
        _enable_pin = SPINDLE_ENABLE_PIN;
#else
        fEnablePin = UNDEFINED_PIN;
#endif

#ifdef SPINDLE_DIR_PIN
        fDirectionPin = SPINDLE_DIR_PIN;
#else
        _direction_pin = UNDEFINED_PIN;
#endif

        if (fOutputPin == UNDEFINED_PIN)
        {
            grbl_msg_sendf(MsgLevel::Info, "Warning: SPINDLE_OUTPUT_PIN not defined");
            return; // We cannot continue without the output pin
        }

        fIsReversable = (fDirectionPin != UNDEFINED_PIN);

        fPWMFrequency = settings_spindle_pwm_freq->get();
        fPWMPrecision = calc_pwm_precision(fPWMFrequency); // detewrmine the best precision
        fPWMPeriod = (1 << fPWMPrecision);

        if (settings_spindle_pwm_min_value->get() > settings_spindle_pwm_min_value->get())
        {
            grbl_msg_sendf(MsgLevel::Info, "Warning: Spindle min pwm is greater than max. Check $35 and $36");
        }

        // pre-calculate some PWM count values
        fPWMOffValue = (fPWMPeriod * settings_spindle_pwm_off_value->get() / 100.0);
        fPWMMinValue = (fPWMPeriod * settings_spindle_pwm_min_value->get() / 100.0);
        fPWMMaxValue = (fPWMPeriod * settings_spindle_pwm_max_value->get() / 100.0);

        fMinRPM = settings_spindle_rpm_min->get();
        fMaxRPM = settings_spindle_rpm_max->get();

        fPWMChannelNumber = 0; // Channel 0 is reserved for spindle use

        fSpinUpDelay = settings_spindle_delay_spinup->get() * 1000.0;
        fSpinDownDelay = settings_spindle_delay_spindown->get() * 1000.0;
    }

    uint32_t PWM::setRPM(uint32_t rpm)
    {
        //  RPM Range Check
        if (rpm > 0)
        {
            rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;
            rpm = (rpm < fMinRPM) ? fMinRPM : rpm;

            // Apply override (percent)
            rpm = static_cast<uint32_t>((static_cast<uint64_t>(rpm) * sys.spindle_speed_ovr) / 100U);
        }
        else
        {
            // Zero RPM Stops the spindle !
            rpm = 0;
        }

        if (fOutputPin == UNDEFINED_PIN)
        {
            return rpm;
        }

        sys.spindle_speed = rpm;

        // Notice: RPM==0 turns PWM off !
        const uint32_t pwmValue = (rpm == 0) ? this->fPWMOffValue : map_uint32_t(rpm, fMinRPM, fMaxRPM, fPWMMinValue, fPWMMaxValue);

        this->set_enable_pin(gc_state.modal.spindle != SpindleState::Disable);
        this->set_output(pwmValue);

        return 0;
    }

    void PWM::setState(SpindleState state, uint32_t rpm)
    {
        if (sys.abort)
        {
            return; // Block during abort.
        }

        if (state == SpindleState::Disable)
        {
            // Halt or set spindle direction and rpm.
            sys.spindle_speed = 0;
            Stop();
            if (fUseDelays && (fCurrentState != state))
            {
                // grbl_msg_sendf(MsgLevel::Info, "Spin down delay");
                delay(fSpinDownDelay);
            }
        }
        else
        {
            set_dir_pin(state == SpindleState::Cw);
            setRPM(rpm);
            set_enable_pin(state != SpindleState::Disable); // must be done after setting rpm for enable features to work
            if (fUseDelays && (fCurrentState != state))
            {
                // grbl_msg_sendf(MsgLevel::Info, "Spin up delay");
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

    void PWM::Stop()
    {
        // inverts are delt with in methods
        set_enable_pin(false);
        set_output(fPWMOffValue);
    }

    void PWM::set_output(uint32_t duty)
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

    void PWM::set_enable_pin(bool enable)
    {
        if (fEnablePin == UNDEFINED_PIN)
        {
            return;
        }

        if (sys.spindle_speed == 0)
        {
            enable = false;
        }

        if (settings_spindle_enable_invert->get())
        {
            enable = !enable;
        }

        digitalWrite(fEnablePin, enable);
    }

    void PWM::set_dir_pin(bool Clockwise) { digitalWrite(fDirectionPin, Clockwise); }

    /*
        Calculate the highest precision of a PWM based on the frequency in bits

        80,000,000 / freq = period
        determine the highest precision where (1 << precision) < period
    */
    uint8_t PWM::calc_pwm_precision(uint32_t freq)
    {
        uint8_t precision = 0;

        // increase the precision (bits) until it exceeds allow by frequency the max or is 16
        while ((1 << precision) < (uint32_t)(80000000 / freq) && precision <= 16)
        {
            precision++;
        }

        return precision - 1;
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
