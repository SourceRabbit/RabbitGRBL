/*
  AlarmsManager.cpp

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

#include "AlarmsManager.h"

AlarmsManager::AlarmsManager()
    : fAlarmNames({
          {EAlarm::None, "No alarm"},
          {EAlarm::HardLimit, "Hard limit"},
          {EAlarm::SoftLimit, "Soft limit"},
          {EAlarm::AbortCycle, "Abort during cycle"},
          {EAlarm::ProbeFailInitial, "Probe fail initial"},
          {EAlarm::ProbeFailContact, "Probe fail contact"},
          {EAlarm::HomingFailReset, "Homing fail reset"},
          {EAlarm::HomingFailDoor, "Homing fail door"},
          {EAlarm::HomingFailPulloff, "Homing fail pulloff"},
          {EAlarm::HomingFailApproach, "Homing fail approach"},
          {EAlarm::SpindleControl, "Spindle control"},
      })
{
}

/**
 * Sends alarm message (ex. ALARM:1) back to client (g-code sender)
 */
void AlarmsManager::ReportAlarmMessage(EAlarm alarm_code)
{
    grbl_sendf("ALARM:%d\r\n", static_cast<int>(alarm_code)); // OK to send to all clients
    delay_ms(500);                                            // Force delay to ensure message clears serial write buffer.
}

/**
 * Sends a list with the Rabbit GRBL alarm codes and titles.
 * If a value is provided, it looks up and prints the specific alarm.
 * If no value is provided, it prints all available alarms.
 *
 * @param value The alarm number as a string, or NULL to list all alarms.
 * @return EError::Ok on success, or EError::InvalidValue if the alarm number is malformed or unknown.
 */
EError AlarmsManager::ListAlarms(const char *value)
{
    if (value)
    {
        // Parse the alarm number from the string
        char *endptr = NULL;
        uint8_t alarmNumber = strtol(value, &endptr, 10);

        // Check if the value is a valid number
        if (*endptr)
        {
            grbl_sendf("Malformed alarm number: %s\r\n", value);
            return EError::InvalidValue;
        }

        // Look up the alarm description
        const char *alarmName = getAlarmTitle(static_cast<EAlarm>(alarmNumber));
        if (alarmName)
        {
            grbl_sendf("%d: %s\r\n", alarmNumber, alarmName);
            return EError::Ok;
        }
        else
        {
            grbl_sendf("Unknown alarm number: %d\r\n", alarmNumber);
            return EError::InvalidValue;
        }
    }

    // No value provided — print all alarms
    for (auto it = fAlarmNames.begin(); it != fAlarmNames.end(); it++)
    {
        grbl_sendf("%d: %s\r\n", static_cast<uint8_t>(it->first), it->second);
    }
    delay_ms(100); // Wait 100ms before sending the "OK"
    return EError::Ok;
}

/**
 * Returns the alarm Title string for the given alarm code, or NULL if not found
 */
const char *AlarmsManager::getAlarmTitle(EAlarm alarmNumber)
{
    auto it = fAlarmNames.find(alarmNumber);
    return it == fAlarmNames.end() ? NULL : it->second;
}
