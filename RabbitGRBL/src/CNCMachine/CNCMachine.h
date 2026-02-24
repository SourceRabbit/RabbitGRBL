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
  along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Motors/MotorsManager.h"
#include "../Spindles/Spindle.h"
#include "Coolant/CoolantManager.h"
#include "Probe/Probe.h"
#include "Backlash/BacklashManager.h"

/**
 * CNCMachine is the central access point for all hardware subsystems.
 * It provides a single Initialize() and Reset() method that manages
 * the entire machine hardware lifecycle.
 */
class CNCMachine
{
public:
    // Initializes all hardware subsystems in the correct order.
    // Must be called once during grbl_init(), after settings are loaded.
    static void Initialize();

    // Resets all hardware subsystems to their safe default state.
    // Called on every system reset (e.g. after receiving Ctrl+X).
    static void Reset();

private:
    // Prevent instantiation - this is a pure static class
    CNCMachine() = delete;
};
