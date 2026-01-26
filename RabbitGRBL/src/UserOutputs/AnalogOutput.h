/*
  AnalogOutput.h

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
  along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdint.h>
#include "UserOutputBase.h"

namespace UserOutput
{
    class AnalogOutput : public UserOutputBase
    {
    public:
        AnalogOutput();
        AnalogOutput(uint8_t number, uint8_t pin, float pwm_frequency);

        bool set_level(uint32_t numerator);
        uint32_t denominator() const { return 1UL << _resolution_bits; }

    protected:
        int8_t _pwm_channel = -1; // -1 means invalid or not setup
        float _pwm_frequency = 0.0f;
        uint8_t _resolution_bits = 0;
        uint32_t _current_value = 0;
    };
}