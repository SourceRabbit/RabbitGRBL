/*
  GRBLCommandsManager.h

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

#include <vector>
#include "../Grbl.h"

class GRBLCommandsManager
{
private:
    // List of all registered GrblCommand objects
    static std::vector<GrblCommand *> fList;

public:
    // Register all GrblCommand objects
    static void Initialize();

    // List all registered commands
    static EError ListCommands(const char *value);

    static EError ShowGRBLHelp(const char *value);

    static EError ShowGRBLBuildInfo(const char *value);

    static EError DoJog(const char *value);

    static EError ToggleCheckMode(const char *value);

    // Home a specific cycle (by cycle bitmask or HOMING_CYCLE_ALL)
    static EError Home(int cycle);

    // Home all axes
    static EError HomeAll(const char *value);

    static EError Execute_ReportNGCParameters(const char *value);
    static EError Execute_ReportGCodeModes(const char *value);

    static EError DisableAlarmLock(const char *value);

    static EError ReportNVSStats(const char *value);

    static EError ReportStartupLines(const char *value);

    static EError SystemSleep(const char *value);

    // Command Checkers
    static bool CommandChecker_AnyState()
    {
        return true;
    }

    static bool CommandChecker_IdleOrJog()
    {
        return sys.state == State::Idle || sys.state == State::Jog;
    }

    static bool CommandChecker_IdleOrAlarm()
    {
        return sys.state == State::Idle || sys.state == State::Alarm;
    }

    static bool CommandChecker_NotCycleOrHold()
    {
        return sys.state != State::Cycle && sys.state != State::Hold;
    }

    // Returns the list of all registered GrblCommand objects
    static std::vector<GrblCommand *> &getGRBLCommandsList()
    {
        return fList;
    }
};
