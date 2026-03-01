/*
  UserOutputsManager.h

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

#include "../../Grbl.h"
#include "UserOutputBase.h"
#include "AnalogOutput.h"
#include "DigitalOutput.h"
#include "UserOutputsManager.h"

const int MaxUserOutputsByType = 4;

class UserOutputsManager
{
public:
    void Initialize();

    AnalogOutput **getMyAnalogOutputs() { return fMyAnalogOutputs; }
    DigitalOutput **getMyDigitalOutputs() { return fMyDigitalOutputs; }

    void TurnAllAnalogOutputsOff();
    void TurnAllDigitalOutputsOff();

    bool SetDigitalOutput(uint8_t io_num, bool turnOn);
    bool SetAnalogOutput(uint8_t io_num, float percent);

private:
    bool fInitialized = false;
    AnalogOutput *fMyAnalogOutputs[MaxUserOutputsByType];
    DigitalOutput *fMyDigitalOutputs[MaxUserOutputsByType];
};
