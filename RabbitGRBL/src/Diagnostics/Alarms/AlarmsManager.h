/*
  AlarmsManager.h

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

#include <map>
#include "../../Grbl.h"
#include "EAlarm.h"
#include "../Errors/EError.h"

class AlarmsManager
{

private:
  static std::map<EAlarm, const char *> fAlarmNames;

public:
  static void ReportAlarmMessage(EAlarm alarm_code);
  static EError ListAlarms(const char *value);
  static const char *getAlarmTitle(EAlarm alarmNumber);
};
