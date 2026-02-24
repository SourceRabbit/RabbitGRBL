/*
  CNCMachine.cpp

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

#include "CNCMachine.h"
#include "../Grbl.h"

/**
 * Initializes all hardware subsystems in the correct order.
 * This centralizes the scattered Initialize() calls from grbl_init().
 */
void CNCMachine::Initialize()
{
    // Initialize backlash compensation first (motors depend on it)
    BacklashManager::Initialize();

    // Initialize motors (steppers for all configured axes)
    MotorsManager::Initialize();

    // Select and initialize the configured spindle type
    Spindles::Spindle::Select();

    // Initialize coolant subsystem (Mist M7 + Flood M8)
    CoolantManager::Initialize();

    // Initialize the probe subsystem
    Probe::Initialize();
}

/**
 * Resets all hardware to a safe state.
 * Called on system reset or emergency stop.
 */
void CNCMachine::Reset()
{
    // Stop spindle immediately
    fSpindle->Stop();

    // Turn off all coolants and re-initialize their state
    CoolantManager::Initialize();

    // Re-initialize probe state
    Probe::Initialize();

    // Reset backlash compensation targets to current position
    BacklashManager::ResetTargets();
}
