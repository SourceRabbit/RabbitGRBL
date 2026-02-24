/*
    Stepper_Software.h

    Copyright (c) 2026 Nikolaos Siatras
    Twitter: nsiatras
    Github: https://github.com/nsiatras
    Website: https://www.sourcerabbit.com

    Rabbit GRBL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rabbit GRBL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Motor.h"

class Stepper_Software : public Motor
{
public:
    Stepper_Software(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

    // Overrides for inherited methods
    void Initialize() override;
    // No special action, but return true to say homing is possible
    bool setHomingMode(bool isHoming) override { return true; }
    void setDisable(bool) override;
    void setDirection(bool) override;
    void Step() override;
    void Unstep() override;

protected:
    bool _invert_step_pin;
    bool _invert_dir_pin;
    uint8_t _step_pin;
    uint8_t _dir_pin;
    uint8_t _disable_pin;
};
