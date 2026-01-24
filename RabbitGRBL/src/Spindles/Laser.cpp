/*
    Laser.cpp

    This is similar to the PWM Spindle except that it allows the
    M4 speed vs. power copensation.

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

*/
#include "Laser.h"

// ===================================== Laser ==============================================

namespace Spindles
{
        bool Laser::inLaserMode()
        {
                return laser_mode->get(); // can use M4 (CCW) laser mode.
        }

        // Get the GPIO from the machine definition
        void Laser::get_pins_and_settings()
        {
                // setup all the pins

#ifdef LASER_OUTPUT_PIN
                _output_pin = LASER_OUTPUT_PIN;
#else
                fOutputPin = UNDEFINED_PIN;
#endif

                fInvertPWM = settings_spindle_output_invert->get();

#ifdef LASER_ENABLE_PIN
                _enable_pin = LASER_ENABLE_PIN;
#else
                fEnablePin = UNDEFINED_PIN;
#endif

                if (fOutputPin == UNDEFINED_PIN)
                {
                        grbl_msg_sendf(MsgLevel::Info, "Warning: LASER_OUTPUT_PIN not defined");
                        return; // We cannot continue without the output pin
                }

                fDirectionPin = UNDEFINED_PIN;
                fIsReversable = false;

                fPWMFrequency = settings_spindle_pwm_freq->get();
                fPWMPrecision = calc_pwm_precision(fPWMFrequency); // detewrmine the best precision
                fPWMPeriod = (1 << fPWMPrecision);

                // pre-caculate some PWM count values
                fPWMOffValue = 0;
                fPWMMinValue = 0;
                fPWMMaxValue = fPWMPeriod;

                fMinRPM = 0;
                fMaxRPM = laser_full_power->get();

                fPWMChannelNumber = 0; // Channel 0 is reserved for spindle use
        }

        void Laser::Dispose()
        {
                Stop();
#ifdef LASER_OUTPUT_PIN
                gpio_reset_pin(LASER_OUTPUT_PIN);
                pinMode(LASER_OUTPUT_PIN, INPUT);
#endif

#ifdef LASER_ENABLE_PIN
                gpio_reset_pin(LASER_ENABLE_PIN);
                pinMode(LASER_ENABLE_PIN, INPUT);
#endif
        }
}
