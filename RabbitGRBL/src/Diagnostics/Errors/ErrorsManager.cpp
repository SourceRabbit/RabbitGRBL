/*
  ErrorsManager.cpp

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

#include "ErrorsManager.h"

std::map<EError, const char *> ErrorsManager::fErrorNames = {
    {EError::Ok, "No error"},
    {EError::ExpectedCommandLetter, "Expected GCodecommand letter"},
    {EError::BadNumberFormat, "Bad GCode number format"},
    {EError::InvalidStatement, "Invalid $ statement"},
    {EError::NegativeValue, "Negative value"},
    {EError::SettingDisabled, "Setting disabled"},
    {EError::SettingStepPulseMin, "Step pulse too short"},
    {EError::SettingReadFail, "Failed to read settings"},
    {EError::IdleError, "Command requires idle state"},
    {EError::SystemGcLock, "GCode cannot be executed in lock or alarm state"},
    {EError::SoftLimitError, "Soft limit error"},
    {EError::Overflow, "Line too long"},
    {EError::MaxStepRateExceeded, "Max step rate exceeded"},
    {EError::CheckDoor, "Check door"},
    {EError::LineLengthExceeded, "Startup line too long"},
    {EError::TravelExceeded, "Max travel exceeded during jog"},
    {EError::InvalidJogCommand, "Invalid jog command"},
    {EError::SettingDisabledLaser, "Laser mode requires PWM output"},
    {EError::HomingNoCycles, "No Homing/Cycle defined in settings"},
    {EError::GcodeUnsupportedCommand, "Unsupported GCode command"},
    {EError::GcodeModalGroupViolation, "Gcode modal group violation"},
    {EError::GcodeUndefinedFeedRate, "Gcode undefined feed rate"},
    {EError::GcodeCommandValueNotInteger, "Gcode command value not integer"},
    {EError::GcodeAxisCommandConflict, "Gcode axis command conflict"},
    {EError::GcodeWordRepeated, "Gcode word repeated"},
    {EError::GcodeNoAxisWords, "Gcode no axis words"},
    {EError::GcodeInvalidLineNumber, "Gcode invalid line number"},
    {EError::GcodeValueWordMissing, "Gcode value word missing"},
    {EError::GcodeUnsupportedCoordSys, "Gcode unsupported coordinate system"},
    {EError::GcodeG53InvalidMotionMode, "Gcode G53 invalid motion mode"},
    {EError::GcodeAxisWordsExist, "Gcode extra axis words"},
    {EError::GcodeNoAxisWordsInPlane, "Gcode no axis words in plane"},
    {EError::GcodeInvalidTarget, "Gcode invalid target"},
    {EError::GcodeArcRadiusError, "Gcode arc radius error"},
    {EError::GcodeNoOffsetsInPlane, "Gcode no offsets in plane"},
    {EError::GcodeUnusedWords, "Gcode unused words"},
    {EError::GcodeG43DynamicAxisError, "Gcode G43 dynamic axis error"},
    {EError::GcodeMaxValueExceeded, "Gcode max value exceeded"},
    {EError::PParamMaxExceeded, "P param max exceeded"},
    {EError::NumberRange, "Number out of range for setting"},
    {EError::InvalidValue, "Invalid value for setting"},
    {EError::NvsSetFailed, "Failed to store setting"},
    {EError::NvsGetStatsFailed, "Failed to get setting status"},
};

/**
 * Sends a list with the Rabbit GRBL error codes and titles.
 * If a value is provided, it looks up and prints the specific error.
 * If no value is provided, it prints all available errors.
 *
 * @param value The error number as a string, or NULL to list all errors.
 * @return EError::Ok on success, or EError::InvalidValue if the error number is malformed or unknown.
 */
EError ErrorsManager::ListErrors(const char *value)
{
    if (value)
    {
        // Parse the error number from the string
        char *endptr = NULL;
        uint8_t errorNumber = strtol(value, &endptr, 10);

        // Check if the parsed value is a valid number (endptr should point to null terminator)
        if (*endptr)
        {
            ConnectionManager::Active().WriteFormatted("Malformed error number: %s\r\n", value);
            return EError::InvalidValue;
        }

        // Look up the error title for the given error number
        const char *errorName = getErrorTitle(static_cast<EError>(errorNumber));
        if (errorName)
        {
            // Error found — send the error number and its title
            ConnectionManager::Active().WriteFormatted("%d: %s\r\n", errorNumber, errorName);
            return EError::Ok;
        }
        else
        {
            // Error number not found in the map
            ConnectionManager::Active().WriteFormatted("Unknown error number: %d\r\n", errorNumber);
            return EError::InvalidValue;
        }
    }

    // No value provided — iterate and print all error codes and their titles
    for (auto it = ErrorsManager::fErrorNames.begin(); it != ErrorsManager::fErrorNames.end(); it++)
    {
        ConnectionManager::Active().WriteFormatted("%d: %s\r\n", static_cast<uint8_t>(it->first), it->second);
    }
    
    delay_ms(100); // Wait 100ms before sending the "OK"
    return EError::Ok;
}

/**
 * Returns the error Title string for the given Error code, or NULL if not found
 */
const char *ErrorsManager::getErrorTitle(EError alarmNumber)
{
    auto it = ErrorsManager::fErrorNames.find(alarmNumber);
    return it == ErrorsManager::fErrorNames.end() ? NULL : it->second;
}
