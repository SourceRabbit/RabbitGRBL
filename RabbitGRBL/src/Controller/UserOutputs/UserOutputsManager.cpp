/*
  UserOutputsManager.cpp

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

#include "UserOutputsManager.h"

/**
 * Initializes the UserOutputsManager
 */
void UserOutputsManager::Initialize()
{
    if (fInitialized)
    {
        return;
    }

    // Setup M62,M63,M64,M65 pins
    fMyDigitalOutputs[0] = new DigitalOutput(0, USER_DIGITAL_PIN_0);
    fMyDigitalOutputs[1] = new DigitalOutput(1, USER_DIGITAL_PIN_1);
    fMyDigitalOutputs[2] = new DigitalOutput(2, USER_DIGITAL_PIN_2);
    fMyDigitalOutputs[3] = new DigitalOutput(3, USER_DIGITAL_PIN_3);

    // Setup M67 Pins
    fMyAnalogOutputs[0] = new AnalogOutput(0, USER_ANALOG_PIN_0, USER_ANALOG_PIN_0_FREQ);
    fMyAnalogOutputs[1] = new AnalogOutput(1, USER_ANALOG_PIN_1, USER_ANALOG_PIN_1_FREQ);
    fMyAnalogOutputs[2] = new AnalogOutput(2, USER_ANALOG_PIN_2, USER_ANALOG_PIN_2_FREQ);
    fMyAnalogOutputs[3] = new AnalogOutput(3, USER_ANALOG_PIN_3, USER_ANALOG_PIN_3_FREQ);

    fInitialized = true;
}
