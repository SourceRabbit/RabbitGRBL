/*
  Coolant.h

  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
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

  /// @brief Initialize coolant
  /// @param pin is the coolant's pin output
  /// @param invertPinOutput  set to true to invert the coolant's output pin from low-disabled/high-enabled to low-enabled/high-disabled.
  /// @param start_delay_setting a FloatSetting defining the coolant's turn on delay in seconds.
  virtual void Initialize(uint8_t pin, bool invertPinOutput, FloatSetting *start_delay_setting);

  /// @brief Turns the coolant On.
  virtual void TurnOn();

  /// @brief Turns the coolant immidiately off.
  virtual void TurnOff();

  /// @brief  Toggle will change the coolant's status. If the coolant is
  // On then it will turn Off, otherwise it will turn On.
  void Toggle();

  /// @brief This method will turn the coolant on or off according to the
  // given state. If state = true then this method will call the
  // TurnOn method otherwise it will call the TurnOff
  /// @param state true (on) or false (off)
  void setState(bool state);

  /// @brief Returns true if the coolant is on
  /// @return
  bool isOn();

protected:
  bool fIsOn = false;
  uint8_t fPinNumber = 0;
  bool fInvertPinOutput = false;
  FloatSetting *fStartDelaySetting;
};
