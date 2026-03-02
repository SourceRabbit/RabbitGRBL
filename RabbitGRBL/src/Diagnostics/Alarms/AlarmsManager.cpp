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

// List of all alarm Diagnostics (id derived from EAlarm enum, title, description)
std::vector<Diagnostic> AlarmsManager::fAlarms = {
    Diagnostic(EAlarm::None, "No alarm", "No alarm has been triggered."),
    Diagnostic(EAlarm::HardLimit, "Hard limit", "Hard limit has been triggered. Machine position is likely lost due to sudden halt. Re-homing is highly recommended."),
    Diagnostic(EAlarm::SoftLimit, "Soft limit", "Soft limit alarm. G-code motion target exceeds machine travel. Machine position retained. Alarm may be safely unlocked."),
    Diagnostic(EAlarm::AbortCycle, "Abort during cycle", "Reset while in motion. Machine position is likely lost due to sudden halt. Re-homing is highly recommended."),
    Diagnostic(EAlarm::ProbeFailInitial, "Probe fail initial", "Probe fail. The probe is not in the expected initial state before starting probe cycle when G38.2 and G38.3 is not triggered and G38.4 and G38.5 is triggered."),
    Diagnostic(EAlarm::ProbeFailContact, "Probe fail contact", "Probe fail. Probe did not contact the workpiece within the programmed travel for G38.2 and G38.4."),
    Diagnostic(EAlarm::HomingFailReset, "Homing fail reset", "Homing fail. The active homing cycle was reset."),
    Diagnostic(EAlarm::HomingFailDoor, "Homing fail door", "Homing fail. Safety door was opened during active homing cycle."),
    Diagnostic(EAlarm::HomingFailPulloff, "Homing fail pulloff", "Homing fail. Homing requires pull-off distance to clear the switch, but none was set in settings."),
    Diagnostic(EAlarm::HomingFailApproach, "Homing fail approach", "Homing fail. Could not find limit switch within search distances. Try increasing max travel, decreasing pull-off distance, or check wiring."),
    Diagnostic(EAlarm::SpindleControl, "Spindle control", "Homing fail. On dual axis machines, could not find the second limit switch for self-squaring."),
};

/**
 * Sends alarm message (ex. ALARM:1) back to client (g-code sender)
 */
void AlarmsManager::ReportAlarmMessage(EAlarm alarm_code)
{
    // Send alarm code to all connected clients
    ConnectionManager::Active().WriteFormatted("ALARM:%d\r\n", static_cast<int>(alarm_code));
    delay_ms(500); // Force delay to ensure message clears serial write buffer
}

/**
 * Sends a list with the Rabbit GRBL alarm codes using the ALARMCODE format.
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
            ConnectionManager::Active().WriteFormatted("Malformed alarm number: %s\r\n", value);
            return EError::InvalidValue;
        }

        // Search the list for the matching alarm ID
        auto it = std::find_if(fAlarms.begin(), fAlarms.end(), [alarmNumber](const Diagnostic &d)
                               { return d.getID() == alarmNumber; });

        if (it != fAlarms.end())
        {
            // Print single alarm in ALARMCODE format
            ConnectionManager::Active().WriteFormatted("[ALARMCODE:%d|%s|%s]\r\n", it->getID(), it->getTitle(), it->getDescription());
            return EError::Ok;
        }
        else
        {
            ConnectionManager::Active().WriteFormatted("Unknown alarm number: %d\r\n", alarmNumber);
            return EError::InvalidValue;
        }
    }
    else
    {
        // Print all alarms in ALARMCODE format
        for (const auto &entry : fAlarms)
        {
            ConnectionManager::Active().WriteFormatted("[ALARMCODE:%d|%s|%s]\r\n", entry.getID(), entry.getTitle(), entry.getDescription());
        }
    }

    delay_ms(100);
    ConnectionManager::Active().Write("ok\r\n");
    return EError::Ok;
}

/**
 * Returns the title string for a given alarm code.
 *
 * @param alarmNumber The alarm enum value.
 * @return The alarm title, or NULL if not found.
 */
const char *AlarmsManager::getAlarmTitle(EAlarm alarmNumber)
{
    uint8_t id = static_cast<uint8_t>(alarmNumber);

    // Search the list for the matching alarm ID
    auto it = std::find_if(fAlarms.begin(), fAlarms.end(), [id](const Diagnostic &d)
                           { return d.getID() == id; });

    if (it != fAlarms.end())
    {
        return it->getTitle();
    }
    return NULL;
}