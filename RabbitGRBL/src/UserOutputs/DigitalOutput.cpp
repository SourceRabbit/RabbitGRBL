/*
  DigitalOutput.cpp

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
#include "../Grbl.h"
#include "DigitalOutput.h"

namespace UserOutput
{
    DigitalOutput::DigitalOutput() {}

    DigitalOutput::DigitalOutput(uint8_t number, uint8_t pin)
        : UserOutputBase(number, pin)
    {
        if (!isValid())
        {
            return;
        }

        // Initialize
        pinMode(this->getPinNumber(), OUTPUT);
        digitalWrite(this->getPinNumber(), LOW);

        grbl_msg_sendf(MsgLevel::Info, "User Digital Output:%d on Pin:%s", this->getNumber(), pinName(this->getPinNumber()).c_str());
    }

    bool DigitalOutput::set_level(bool isOn)
    {
        // Keep original behavior: if output is "not configured", reject turning it on.
        if (this->getPinNumber() == UNDEFINED_PIN && isOn)
        {
            return false;
        }

        if (!isValid())
        {
            return false;
        }

        digitalWrite(this->getPinNumber(), isOn);
        return true;
    }
}