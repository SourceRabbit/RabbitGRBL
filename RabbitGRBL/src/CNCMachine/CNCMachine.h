/*
  CNCMachine.h

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
  along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "../Grbl.h"
#include "Backlash/BacklashManager.h"
#include "Peripherals/Motors/MotorsManager.h"
#include "Peripherals/Coolant/CoolantManager.h"
#include "Peripherals/Probe/Probe.h"

class CNCMachine
{
public:
  static BacklashManager fBacklashManager;
  static MotorsManager fMotorsManager;
  static CoolantManager fCoolantManager;
  static Probe fProbe;

  static void Initialize();
  static void Reset();
};
