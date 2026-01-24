/*
    Spindle.cpp

    A Base class for spindles and spinsle like things such as lasers

    Part of Grbl_ESP32

    2020 -  Bart Dring

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

    TODO
        Add Spindle spin up/down delays

    Get rid of dependance on machine definition #defines
        SPINDLE_OUTPUT_PIN
        SPINDLE_ENABLE_PIN
        SPINDLE_DIR_PIN

*/
#include "Spindle.h"

#include "NullSpindle.h"
#include "PWMSpindle.h"
#include "RelaySpindle.h"
#include "Laser.h"
#include "BESCSpindle.h"

// Declares the machine's spindle
Spindles::Spindle *fSpindle;


namespace Spindles
{
    // An instance of each type of spindle is created here.
    // This allows the spindle to be dynamicly switched
    Null null;
    PWM pwm;
    Relay relay;
    Laser laser;
    BESC besc;

    void Spindle::Select()
    {
        switch (static_cast<ESpindleType>(settings_spindle_type->get()))
        {
        case ESpindleType::PWM:
            fSpindle = &pwm;
            break;
        case ESpindleType::RELAY:
            fSpindle = &relay;
            break;
        case ESpindleType::LASER:
            fSpindle = &laser;
            break;
        case ESpindleType::BESC:
            fSpindle = &besc;
            break;
        case ESpindleType::NONE:
        default:
            fSpindle = &null;
            break;
        }

        fSpindle->Initialize();
    }

    // ========================= Spindle ==================================

    bool Spindle::inLaserMode()
    {
        return false; // default for basic spindle is false
    }

    void Spindle::Synch(SpindleState state, uint32_t rpm)
    {
        if (sys.state == State::CheckMode)
        {
            return;
        }
        protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
        setState(state, rpm);
    }

    void Spindle::Dispose() { Stop(); }
}


