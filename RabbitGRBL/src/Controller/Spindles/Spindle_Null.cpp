/*
    Spindle_Null.cpp

    This is used when you don't want to use a spindle No I/O will be used
    and most methods don't do anything

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
    along with Grbl. If not, see <http://www.gnu.org/licenses/>.

*/
#include "Spindle_Null.h"

// ======================= Spindle_Null ==============================
// Spindle_Null is just bunch of do nothing (ignore) methods to be used when you don't want a spindle

void Spindle_Null::Initialize()
{
}

uint32_t Spindle_Null::setRPM(uint32_t rpm)
{
    sys.spindle_speed = rpm;
    return rpm;
}

void Spindle_Null::setState(SpindleState state, uint32_t rpm)
{
    fCurrentState = state;
    sys.spindle_speed = rpm;
}

SpindleState Spindle_Null::getState()
{
    return fCurrentState;
}

void Spindle_Null::Stop()
{
    // Do nothing
}

bool Spindle_Null::isReversable()
{
    return false;
}
