#pragma once

/*
    Spindle_Null.h

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
#include "Spindle.h"

// This is a dummy spindle that has no I/O.
// It is used to ignore spindle commands when no spinde is desired
class Spindle_Null : public Spindle
{
public:
    Spindle_Null() = default;

    Spindle_Null(const Spindle_Null &) = delete;
    Spindle_Null(Spindle_Null &&) = delete;
    Spindle_Null &operator=(const Spindle_Null &) = delete;
    Spindle_Null &operator=(Spindle_Null &&) = delete;

    void Initialize() override;
    uint32_t setRPM(uint32_t rpm) override;
    void setState(SpindleState state, uint32_t rpm) override;
    SpindleState getState() override;
    void Stop() override;
    bool isReversable() override;

    virtual ~Spindle_Null() {}
};
