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
 * Initializes the UserOutputsManager.
 * Sets up all analog (M67) and digital (M62/M63/M64/M65) output pins.
 * This method is safe to call multiple times; it will only initialize once.
 */
void UserOutputsManager::Initialize()
{
    if (fInitialized)
    {
        return;
    }

    // Setup M67 Pins
    fMyAnalogOutputs[0] = new AnalogOutput(0, USER_ANALOG_PIN_0, USER_ANALOG_PIN_0_FREQ);
    fMyAnalogOutputs[1] = new AnalogOutput(1, USER_ANALOG_PIN_1, USER_ANALOG_PIN_1_FREQ);
    fMyAnalogOutputs[2] = new AnalogOutput(2, USER_ANALOG_PIN_2, USER_ANALOG_PIN_2_FREQ);
    fMyAnalogOutputs[3] = new AnalogOutput(3, USER_ANALOG_PIN_3, USER_ANALOG_PIN_3_FREQ);

    // Setup M62,M63,M64,M65 pins
    fMyDigitalOutputs[0] = new DigitalOutput(0, USER_DIGITAL_PIN_0);
    fMyDigitalOutputs[1] = new DigitalOutput(1, USER_DIGITAL_PIN_1);
    fMyDigitalOutputs[2] = new DigitalOutput(2, USER_DIGITAL_PIN_2);
    fMyDigitalOutputs[3] = new DigitalOutput(3, USER_DIGITAL_PIN_3);

    fInitialized = true;
}

/**
 * Sets the state of a digital output pin.
 * @param io_num  The index of the digital output (0-based).
 * @param turnOn  True to turn the output ON, false to turn it OFF.
 * @return        True if the operation was successful, false otherwise.
 */
bool UserOutputsManager::SetDigitalOutput(uint8_t io_num, bool turnOn)
{
    return this->getMyDigitalOutputs()[io_num]->set_level(turnOn);
}

/**
 * Sets the level of an analog output pin based on a percentage value.
 * @param io_num   The index of the virtual analog output pin (0-based).
 * @param percent  The desired output level as a percentage (0.0 to 100.0).
 * @return         True if the operation was successful, false otherwise.
 */
bool UserOutputsManager::SetAnalogOutput(uint8_t io_num, float percent)
{
    auto analog = this->getMyAnalogOutputs()[io_num];
    uint32_t numerator = percent / 100.0 * analog->denominator();
    return analog->set_level(numerator);
}

/**
 * Turns off all analog outputs by setting their level to 0.
 * Iterates over all available analog output slots and resets each one.
 */
void UserOutputsManager::TurnAllAnalogOutputsOff()
{
    auto outputs = this->getMyAnalogOutputs();
    for (uint8_t io_num = 0; io_num < MaxUserOutputsByType; io_num++)
    {
        outputs[io_num]->set_level(0);
    }
}

/**
 * Turns off all digital outputs by setting their level to LOW.
 * Iterates over all available digital output slots and resets each one.
 */
void UserOutputsManager::TurnAllDigitalOutputsOff()
{
    auto outputs = this->getMyDigitalOutputs();
    for (uint8_t io_num = 0; io_num < MaxUserOutputsByType; io_num++)
    {
        outputs[io_num]->set_level(LOW);
    }
}