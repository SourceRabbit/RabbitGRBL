/*
  SettingsManager.h

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

class SettingsManager
{
public:
    // Initialize the settings subsystem: register settings, commands and load from NVS
    static void Initialize();

    // Register all setting objects (calls extern void make_settings())
    static void MakeSettings();

    // Load all setting values from NVS into memory
    static void LoadSettings();

    // Restore settings to defaults based on restore_flag bitmask
    static void RestoreSettings(uint8_t restore_flag);

    // Parse and execute a full $ or [...] command line from the user
    static EError ExecuteLine(char *line);

    // Execute the stored GCode startup lines (Line0 and Line1)
    static void ExecuteStartupLines(char *line);

    // Report all standard GRBL settings ($xx)
    static EError ReportNormalSettings(const char *value);

    // Report all extended settings
    static EError ReportExtendedSettings(const char *value);

    // List all GRBL-compatible name mappings
    static EError ListGrblNames(const char *value);

    // List all settings with their current values
    static EError ListSettings(const char *value);

    // List only settings that differ from their default values
    static EError ListChangedSettings(const char *value);

    // Restore settings via user command — GrblCommand callback wrapper
    static EError RestoreSettingsCommand(const char *value);

private:
    // Format and send a single setting line to the active connection
    static void ShowSetting(const char *name, const char *value, const char *description);

    // Send all settings of a given type (axis or non-axis) to the active connection
    static void ShowGrblSettings(ERabbitGRBLItemType type, bool wantAxis);

    // Parse a normalized key/value pair and dispatch to the matching setting or command
    static EError ExecuteCommandOrSetting(const char *key, char *value);

    // Strip leading whitespace and null-terminate at the first trailing whitespace
    static char *NormalizeKey(char *start);
};
