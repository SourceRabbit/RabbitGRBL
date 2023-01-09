/*
  Coolant.cpp

  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Website: https://www.sourcerabbit.com

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../Grbl.h"
#include "Coolant.h"

Coolant::Coolant()
{
}

void Coolant::Initialize(uint8_t pin, bool invertPinOutput)
{
    fPinNumber = pin;
    fInvertPinOutput = invertPinOutput;

    if (fPinNumber > 0)
    {
        pinMode(fPinNumber, OUTPUT);
    }

    // Turn off after initialization
    this->TurnOff();
}

/**
 * Turn the coolant On.
 */
void Coolant::TurnOn()
{
    if (fPinNumber > 0)
    {
        digitalWrite(fPinNumber, (fInvertPinOutput) ? 0 : 1);
        fIsOn = true;
    }
}

/**
 * Turn the coolant On and wait.
 */
void Coolant::TurnOnWithDelay(uint16_t delayMilliseconds)
{
    if (fPinNumber > 0)
    {
        this->TurnOn();

        // delay_msec(int32_t(1000.0 * coolant_flood_start_delay->get()), DwellMode::SysSuspend);
        delay_msec(delayMilliseconds, DwellMode::SysSuspend);
        // delay(delayMilliseconds);
    }
}

/**
 * Turn the coolant Off.
 */
void Coolant::TurnOff()
{
    if (fPinNumber > 0)
    {
        digitalWrite(fPinNumber, (fInvertPinOutput) ? 1 : 0);
    }
    fIsOn = false;
}

/**
 * Toggle will change the coolant's status. If the coolant is On then
 * it will turn Off, otherwise it will turn On.
 */
void Coolant::Toggle()
{
    if (this->isOn())
    {
        this->TurnOff();
    }
    else
    {
        this->TurnOn();
    }

    sys.report_ovr_counter = 0; // Set to report change immediately
}

/**
 * This method will turn the coolant on or off according to the
 * given state. If state = true then this method will call the
 * TurnOn method otherwise it will call the TurnOff
 */
void Coolant::setState(bool state)
{
    if (state)
    {
        this->TurnOn();
    }
    else
    {
        this->TurnOn();
    }
}

/**
 * Returns true if the Coolant is on
 */
bool Coolant::isOn()
{
    return fIsOn;
}