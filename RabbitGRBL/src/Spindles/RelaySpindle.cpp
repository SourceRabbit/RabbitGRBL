/*
    RelaySpindle.cpp

    This is used for a basic on/off spindle All S Values above 0
    will turn the spindle on.

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
#include "RelaySpindle.h"

// ========================= Relay ==================================

namespace Spindles
{
    /*
    This is a sub class of PWM but is a digital rather than PWM output
*/
    void Relay::Initialize()
    {
        get_pins_and_settings();

        if (fOutputPin == UNDEFINED_PIN)
        {
            return;
        }

        pinMode(fOutputPin, OUTPUT);
        pinMode(fEnablePin, OUTPUT);
        pinMode(fDirectionPin, OUTPUT);

        fIsReversable = (fDirectionPin != UNDEFINED_PIN);
        fUseDelays = true;

    }


    uint32_t Relay::setRPM(uint32_t rpm)
    {
        // Max RPM security check
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;

        if (fOutputPin == UNDEFINED_PIN)
        {
            return rpm;
        }

        sys.spindle_speed = rpm;
        set_output(rpm != 0);

        return rpm;
    }

    void Relay::set_output(uint32_t duty)
    {
#ifdef INVERT_SPINDLE_PWM
        duty = (duty == 0); // flip duty
#endif
        digitalWrite(fOutputPin, duty > 0); // anything greater
    }
}
