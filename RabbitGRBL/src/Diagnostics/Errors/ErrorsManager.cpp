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

// List of all error Diagnostics (id derived from EError enum, title, description)
std::vector<Diagnostic> ErrorsManager::fErrors = {
    Diagnostic(EError::Ok, "No error", "No error."),
    Diagnostic(EError::ExpectedCommandLetter, "Expected command letter", "G-code words consist of a letter and a value. Letter was not found."),
    Diagnostic(EError::BadNumberFormat, "Bad number format", "Missing the expected G-code word value or numeric value format is not valid."),
    Diagnostic(EError::InvalidStatement, "Invalid statement", "Grbl '$' system command was not recognized or supported."),
    Diagnostic(EError::NegativeValue, "Negative value", "Negative value received for an expected positive-only parameter."),
    Diagnostic(EError::SettingDisabled, "Setting disabled", "Homing cycle failure. Homing is not enabled via settings."),
    Diagnostic(EError::SettingStepPulseMin, "Step pulse too short", "Minimum step pulse time must be greater than 3usec."),
    Diagnostic(EError::SettingReadFail, "Failed to read settings", "EEPROM read failed. Reset and restored to default values."),
    Diagnostic(EError::IdleError, "Command requires idle state", "Grbl '$' command cannot be used unless Grbl is IDLE. Ensure no motions are active and try again."),
    Diagnostic(EError::SystemGcLock, "GCode locked", "G-code commands are locked out during alarm or jog state."),
    Diagnostic(EError::SoftLimitError, "Soft limit error", "Soft limits cannot be enabled without homing also enabled."),
    Diagnostic(EError::Overflow, "Line too long", "Max characters per line exceeded. Received command line was not executed."),
    Diagnostic(EError::MaxStepRateExceeded, "Max step rate exceeded", "Grbl '$' setting value cause the step rate to exceed the maximum supported."),
    Diagnostic(EError::CheckDoor, "Check door", "Safety door detected as opened and door state initiated."),
    Diagnostic(EError::LineLengthExceeded, "Startup line too long", "Build info or startup line exceeded EEPROM line length limit. Line not stored."),
    Diagnostic(EError::TravelExceeded, "Max travel exceeded during jog", "Jog target exceeds machine travel. Jog command has been ignored."),
    Diagnostic(EError::InvalidJogCommand, "Invalid jog command", "Jog command has no '=' or contains prohibited g-code."),
    Diagnostic(EError::SettingDisabledLaser, "Laser mode requires PWM output", "Laser mode requires PWM output."),
    Diagnostic(EError::HomingNoCycles, "No homing cycle defined", "No Homing/Cycle defined in settings."),
    Diagnostic(EError::GcodeUnsupportedCommand, "Unsupported GCode command", "Unsupported or invalid g-code command found in block."),
    Diagnostic(EError::GcodeModalGroupViolation, "GCode modal group violation", "More than one g-code command from same modal group found in block."),
    Diagnostic(EError::GcodeUndefinedFeedRate, "GCode undefined feed rate", "Feed rate has not yet been set or is undefined."),
    Diagnostic(EError::GcodeCommandValueNotInteger, "GCode command value not integer", "G-code command requires an integer value."),
    Diagnostic(EError::GcodeAxisCommandConflict, "GCode axis command conflict", "Two G-code commands that both require the use of the XYZ axis words were detected in the block."),
    Diagnostic(EError::GcodeWordRepeated, "GCode word repeated", "A G-code word was repeated in the block."),
    Diagnostic(EError::GcodeNoAxisWords, "GCode no axis words", "A G-code command implicitly or explicitly requires XYZ axis words in the block, but none were detected."),
    Diagnostic(EError::GcodeInvalidLineNumber, "GCode invalid line number", "Line number value given is greater than the maximum line number allowed."),
    Diagnostic(EError::GcodeValueWordMissing, "GCode value word missing", "G-code command is missing a required value word."),
    Diagnostic(EError::GcodeUnsupportedCoordSys, "GCode unsupported coord system", "G59.x work coordinate systems are not supported."),
    Diagnostic(EError::GcodeG53InvalidMotionMode, "GCode G53 invalid motion mode", "G53 only allowed with G0 and G1 motion modes."),
    Diagnostic(EError::GcodeAxisWordsExist, "GCode extra axis words", "Axis words found in block when no command or current modal state uses them."),
    Diagnostic(EError::GcodeNoAxisWordsInPlane, "GCode no axis words in plane", "G2 and G3 arcs require at least one in-plane axis word."),
    Diagnostic(EError::GcodeInvalidTarget, "GCode invalid target", "Motion command target is invalid."),
    Diagnostic(EError::GcodeArcRadiusError, "GCode arc radius error", "Arc radius value is invalid."),
    Diagnostic(EError::GcodeNoOffsetsInPlane, "GCode no offsets in plane", "G2 and G3 arcs require at least one in-plane offset word."),
    Diagnostic(EError::GcodeUnusedWords, "GCode unused words", "Unused value words found in block."),
    Diagnostic(EError::GcodeG43DynamicAxisError, "GCode G43 dynamic axis error", "G43.1 dynamic tool length offset is not assigned to configured tool length axis."),
    Diagnostic(EError::GcodeMaxValueExceeded, "GCode max value exceeded", "Tool number greater than max supported value."),
    Diagnostic(EError::PParamMaxExceeded, "P param max exceeded", "P command parameter value exceeds the maximum value."),
    Diagnostic(EError::NumberRange, "Setting number range problem", "Value out of range for the given setting."),
    Diagnostic(EError::InvalidValue, "Invalid value", "Setting string is invalid."),
    Diagnostic(EError::NvsSetFailed, "NVS set failed", "Failed to write value to NVS storage."),
    Diagnostic(EError::NvsGetStatsFailed, "NVS get stats failed", "Failed to read NVS storage statistics."),
    Diagnostic(EError::AuthenticationFailed, "Authentication failed", "Authentication failed."),
    Diagnostic(EError::Eol, "End of line", "End of line marker encountered."),
};

/**
 * Sends a list with the Rabbit GRBL error codes using the ERRORCODE format.
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

        // Check if the value is a valid number
        if (*endptr)
        {
            ConnectionManager::Active().WriteFormatted("Malformed error number: %s\r\n", value);
            return EError::InvalidValue;
        }

        // Search the list for the matching error ID
        auto it = std::find_if(fErrors.begin(), fErrors.end(), [errorNumber](const Diagnostic &d)
                               { return d.getID() == errorNumber; });

        if (it != fErrors.end())
        {
            // Print single error in ERRORCODE format
            ConnectionManager::Active().WriteFormatted("[ERRORCODE:%d|%s|%s]\r\n", it->getID(), it->getTitle(), it->getDescription());
            return EError::Ok;
        }
        else
        {
            ConnectionManager::Active().WriteFormatted("Unknown error number: %d\r\n", errorNumber);
            return EError::InvalidValue;
        }
    }
    else
    {
        // Print all errors in ERRORCODE format
        for (const auto &entry : fErrors)
        {
            ConnectionManager::Active().WriteFormatted("[ERRORCODE:%d|%s|%s]\r\n", entry.getID(), entry.getTitle(), entry.getDescription());
        }
    }

    delay_ms(100);
    return EError::Ok;
}

/**
 * Returns the title string for a given error code.
 *
 * @param errorNumber The error enum value.
 * @return The error title, or NULL if not found.
 */
const char *ErrorsManager::getErrorTitle(EError errorNumber)
{
    uint8_t id = static_cast<uint8_t>(errorNumber);

    // Search the list for the matching error ID
    auto it = std::find_if(fErrors.begin(), fErrors.end(), [id](const Diagnostic &d)
                           { return d.getID() == id; });

    if (it != fErrors.end())
    {
        return it->getTitle();
    }
    return NULL;
}