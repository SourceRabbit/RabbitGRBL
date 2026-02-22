/*
  UserOutputBase.h

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
  Website: https://www.sourcerabbit.com

  Rabbit Rabbit GRBL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Rabbit Rabbit GRBL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include "../Grbl.h"
#include <stdint.h>

namespace UserOutput
{
    // Base class for all user outputs (digital/analog).
    // It stores the common output identity (number) and the assigned MCU pin.
    class UserOutputBase
    {
    public:
        UserOutputBase() = default;

        /// @brief Constructs a user output with the given logical output number and assigned MCU pin.
        /// @param number Logical user output index (e.g. 0..MaxUserDigitalPin-1).
        /// @param pin MCU GPIO number used for this output. Use UNDEFINED_PIN if not configured.
        UserOutputBase(uint8_t number, uint8_t pin)
        {
            fNumber = number;
            fPinNumber = pin;
        }

        virtual ~UserOutputBase() = default;

        /// @brief Returns true when this output has a valid configured pin.
        /// @return
        bool isValid() const
        {
            return fPinNumber != UNDEFINED_PIN;
        }

        uint8_t getNumber() const { return fNumber; }
        uint8_t getPinNumber() const { return fPinNumber; }

    protected:
        uint8_t fNumber = UNDEFINED_PIN;
        uint8_t fPinNumber = UNDEFINED_PIN;
    };
}