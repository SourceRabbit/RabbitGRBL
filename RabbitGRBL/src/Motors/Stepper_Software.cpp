/*
    Stepper_Software.cpp

    This is used for a stepper motor that just requires step and direction
    pins, using software (GPIO) to generate step pulses.

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

#include "Stepper_Software.h"

#ifndef USE_RMT_STEPS

namespace Motors
{
    Stepper_Software::Stepper_Software(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin)
        : Motor(axis_index), _step_pin(step_pin), _dir_pin(dir_pin), _disable_pin(disable_pin)
    {
    }

    void Stepper_Software::Initialize()
    {
        _invert_step_pin = bitnum_istrue(step_invert_mask->get(), _axis_index);
        _invert_dir_pin = bitnum_istrue(dir_invert_mask->get(), _axis_index);
        pinMode(_dir_pin, OUTPUT);
        pinMode(_step_pin, OUTPUT);
        pinMode(_disable_pin, OUTPUT);

        // Information Message
        MessageSender::SendMessage(EMessageLevel::Info,
                                   "%s Software Stepper Step:%s Dir:%s Disable:%s %s",
                                   reportAxisNameMsg(_axis_index, _dual_axis_index),
                                   pinName(_step_pin).c_str(),
                                   pinName(_dir_pin).c_str(),
                                   pinName(_disable_pin).c_str(),
                                   reportAxisLimitsMsg(_axis_index));
    }

    void Stepper_Software::step()
    {
        digitalWrite(_step_pin, !_invert_step_pin);
    }

    void Stepper_Software::unstep()
    {
        digitalWrite(_step_pin, _invert_step_pin);
    }

    void Stepper_Software::set_direction(bool dir) { digitalWrite(_dir_pin, dir ^ _invert_dir_pin); }

    void Stepper_Software::set_disable(bool disable)
    {
        digitalWrite(_disable_pin, disable);
    }
}

#endif // USE_RMT_STEPS
