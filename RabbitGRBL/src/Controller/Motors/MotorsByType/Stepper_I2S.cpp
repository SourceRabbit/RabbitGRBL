/*
    Stepper_I2S.cpp

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

#include "Stepper_I2S.h"

Stepper_I2S::Stepper_I2S(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin)
    : Motor(axis_index),
      fStepPin(step_pin),
      fDirPin(dir_pin),
      fDisablePin(disable_pin)
{
}

void Stepper_I2S::Initialize()
{
    // Read inversion masks from GRBL settings
    fInvertStepPin = bitnum_istrue(step_invert_mask->get(), fAxisIndex);
    fInvertDirPin = bitnum_istrue(dir_invert_mask->get(), fAxisIndex);

    // Set all virtual pins to their safe idle state
    i2sOutWrite(fStepPin, fInvertStepPin ? 1 : 0); // Step idle (inactive level)
    i2sOutWrite(fDirPin, 0);                       // Direction default low
    i2sOutWrite(fDisablePin, 1);                   // Motors disabled at startup

    MessageSender::SendMessage(EMessageLevel::Info,
                               "%s I2S Stepper Step:I2SO(%d) Dir:I2SO(%d) Disable:I2SO(%d) %s",
                               reportAxisNameMsg(fAxisIndex, fDualAxisIndex),
                               fStepPin,
                               fDirPin,
                               fDisablePin,
                               reportAxisLimitsMsg(fAxisIndex));
}

void Stepper_I2S::Step()
{
    // Assert step pin to active level
    i2sOutWrite(fStepPin, fInvertStepPin ? 0 : 1);
}

void Stepper_I2S::Unstep()
{
    // Return step pin to idle (inactive) level
    i2sOutWrite(fStepPin, fInvertStepPin ? 1 : 0);
}

void Stepper_I2S::setDirection(bool dir)
{
    i2sOutWrite(fDirPin, dir ^ fInvertDirPin);
}

void Stepper_I2S::setDisable(bool disable)
{
    i2sOutWrite(fDisablePin, disable ? 1 : 0);
}
