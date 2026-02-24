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
  along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../Grbl.h"
#include "CNCMachine.h"

BacklashManager CNCMachine::fBacklashManager;
MotorsManager CNCMachine::fMotorsManager;
CoolantManager CNCMachine::fCoolantManager;
Probe CNCMachine::fProbe;
AlarmsManager CNCMachine::fAlarmsManager;
ErrorsManager CNCMachine::fErrorsManager;

/**
 * Initialize the CNC Machine
 */
void CNCMachine::Initialize()
{
    CNCMachine::fBacklashManager.Initialize();
    CNCMachine::fMotorsManager.Initialize();
    CNCMachine::fCoolantManager.Initialize();
    CNCMachine::fProbe.Initialize();
}

/**
 * Reset the CNC Machine.
 * (Re-Initialize managers etc.)
 */
void CNCMachine::Reset()
{
    CNCMachine::fBacklashManager.ResetTargets();
    CNCMachine::fCoolantManager.Initialize();
    CNCMachine::fProbe.Initialize();
}
