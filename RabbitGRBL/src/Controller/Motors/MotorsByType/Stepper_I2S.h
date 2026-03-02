/*
    Stepper_I2S.h

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

#include "../../../Grbl.h"
#include "../Motor.h"
#include "../../../I2SOut.h"

// Each virtual I2S pin maps to one bit in the 32-bit shift register word.
// Assign 3 bits per axis: DISABLE | DIR | STEP
// Axis 0 (X) -> bits 0,1,2 | Axis 1 (Y) -> bits 4,5,6 | etc.
#define I2S_AXIS_BIT_OFFSET 4 // bits per axis slot

class Stepper_I2S : public Motor
{
public:
    Stepper_I2S(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

    // Overrides for inherited methods
    void Initialize() override;
    bool setHomingMode(bool isHoming) override { return true; }
    void setDisable(bool disable) override;
    void setDirection(bool dir) override;
    void Step() override;
    void Unstep() override;

protected:
    // Virtual pin numbers: these index into the I2S shift register word (0..31)
    uint8_t fStepPin;    // Virtual step pin (I2S bit index)
    uint8_t fDirPin;     // Virtual direction pin (I2S bit index)
    uint8_t fDisablePin; // Virtual disable pin (I2S bit index)

    bool fInvertStepPin; // True if step polarity is inverted
    bool fInvertDirPin;  // True if direction polarity is inverted
};
