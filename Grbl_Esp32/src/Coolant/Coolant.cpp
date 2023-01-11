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

void Coolant::Initialize(uint8_t pin, bool invertPinOutput, FloatSetting *start_delay_setting)
{
    this->fPinNumber = pin;
    this->fInvertPinOutput = invertPinOutput;
    this->fStartDelaySetting = start_delay_setting;

    if (this->fPinNumber > 0)
    {
        pinMode(this->fPinNumber, OUTPUT);
    }

    // Turn off after initialization
    this->TurnOff();
}

void Coolant::TurnOn()
{
    if (this->fPinNumber > 0)
    {
        digitalWrite(this->fPinNumber, (this->fInvertPinOutput) ? 0 : 1);
        this->fIsOn = true;
    }
}

void Coolant::TurnOnWithDelay()
{
    if (this->fPinNumber > 0)
    {
        this->TurnOn();
        delay_msec((1000.0 * this->fStartDelaySetting->get()), DwellMode::SysSuspend);
    }
}

void Coolant::TurnOff()
{
    if (this->fPinNumber > 0)
    {
        digitalWrite(this->fPinNumber, (this->fInvertPinOutput) ? 1 : 0);
    }
    this->fIsOn = false;
}

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
    return this->fIsOn;
}