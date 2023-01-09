/*
  Coolant.h

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

#pragma once

#include <Arduino.h>
#include <cstdint>

class Coolant
{

public:
  Coolant();

  virtual void Initialize(uint8_t pin, bool invertPinOutput);

  virtual void TurnOn();
  virtual void TurnOnWithDelay(uint16_t delayMilliseconds);
  virtual void TurnOff();
  void Toggle();
  void setState(bool state);

  bool isOn();

protected:
  bool fIsOn = false;
  uint8_t fPinNumber = 0;
  bool fInvertPinOutput = false;
};
