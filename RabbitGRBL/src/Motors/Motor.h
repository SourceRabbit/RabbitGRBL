#pragma once

/*
    Motor.h
    Header file for Motor Classes
    Here is the hierarchy
        Motor
            Nullmotor
            Stepper_RMT
            Stepper_Software

    These are for motors coordinated by Grbl_ESP32
    See motorClass.cpp for more details

    Part of Grbl_ESP32
    2020 -	Bart Dring

    Rabbit GRBL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Rabbit GRBL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Motors.h"

#include <cstdint>

namespace Motors
{
    class Motor
    {
    public:
        Motor(uint8_t axis_index);

        // Initialize() establishes configured motor parameters.  It is called after
        // all motor objects have been constructed.
        virtual void Initialize() {}

        // debug_message() displays motor-specific information that can be
        // used to assist with motor configuration.  For many motor types,
        // it is a no-op.
        virtual void debug_message();

        // setHomingMode() is called from motors_set_homing_mode(),
        // which in turn is called at the beginning of a homing cycle
        // with isHoming true, and at the end with isHoming false.
        // Some motor types require differ setups for homing and
        // normal operation.  Returns true if the motor can home
        virtual bool setHomingMode(bool isHoming) = 0;

        // setDisable() disables or enables a motor.  It is used to
        // make a motor transition between idle and non-idle states.
        virtual void setDisable(bool disable) {}

        // setDirection() sets the motor movement direction.  It is
        // invoked for every motion segment.
        virtual void setDirection(bool) {}

        // Step() initiates a step operation on a motor.  It is called
        // from motors_step() for ever motor than needs to step now.
        // For ordinary step/direction motors, it sets the step pin
        // to the active state.
        virtual void Step() {}

        // Unstep() turns off the step pin, if applicable, for a motor.
        // It is called from motors_unstep() for all motors, since
        // motors_unstep() is used in many contexts where the previous
        // states of the step pins are unknown.
        virtual void Unstep() {}

        // Update() is used for some types of "smart" motors that
        // can be told to move to a specific position.  It is
        // called from a periodic task.
        virtual void Update() {}

    protected:
        // fDualAxisIndex is 0 for the primary motor on that
        // axis and 1 for the ganged motor.
        // These variables are used for several purposes:
        // * Displaying the axis name in messages
        // * When reading settings, determining which setting
        //   applies to this motor
        // * For some motor types, it is necessary to maintain
        //   tables of all the motors of that type; those
        //   tables can be indexed by these variables.
        // TODO Architecture: It might be useful to cache a
        // reference to the axis settings entry.
        uint8_t fAxisIndex;      // X_AXIS, etc
        uint8_t fDualAxisIndex; // 0 = primary 1=ganged
    };
}
