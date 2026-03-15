/*
  SettingsManager.cpp

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

#include "SettingsManager.h"

#include <map>

static std::map<const char *, uint8_t, cmp_str> fRestoreCommands =
    {

#ifdef ENABLE_RESTORE_DEFAULT_SETTINGS
        {"$", SettingsRestore::Defaults},
        {"settings", SettingsRestore::Defaults},
#endif

#ifdef ENABLE_RESTORE_CLEAR_PARAMETERS
        {"#", SettingsRestore::Parameters},
        {"gcode", SettingsRestore::Parameters},
#endif

#ifdef ENABLE_RESTORE_WIPE_ALL
        {"*", SettingsRestore::All},
        {"all", SettingsRestore::All},
#endif
};

void SettingsManager::Initialize()
{
    MakeSettings();

    LoadSettings();
}

void SettingsManager::ShowSetting(const char *name, const char *value, const char *description)
{
    // Build the full setting string before sending it.
    char setting[RX_BUFFER_SIZE];

    if (description)
    {
        // Format: "$name=value    description\r\n"
        snprintf(setting, sizeof(setting), "$%s=%s    %s\r\n", name, value, description);
    }
    else
    {
        // Format: "$name=value\r\n"
        snprintf(setting, sizeof(setting), "$%s=%s\r\n", name, value);
    }

    ConnectionManager::Write(setting);
}

void SettingsManager::ShowGrblSettings(ERabbitGRBLItemType type, bool wantAxis)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        if (s->getType() == type && s->getGrblName())
        {
            bool isAxis = s->getAxis() != NO_AXIS;
            // The following test could be expressed more succinctly with XOR,
            // but is arguably clearer when written out
            if ((wantAxis && isAxis) || (!wantAxis && !isAxis))
            {
                ShowSetting(s->getGrblName(), s->getCompatibleValue(), NULL);
            }
        }
    }
}

// NormalizeKey puts a key string into canonical form - without whitespace.
// start points to a null-terminated string.
// Returns the first substring that does not contain whitespace.
// Case is unchanged because comparisons are case-insensitive.
char *SettingsManager::NormalizeKey(char *start)
{
    // Skip leading whitespace
    while (*start != '\0' && isspace(*start))
    {
        ++start;
    }

    // Terminate at the first trailing whitespace
    char *end = start;
    while (*end != '\0' && !isspace(*end))
    {
        ++end;
    }
    *end = '\0';

    return start;
}

// ExecuteCommandOrSetting is the handler for all forms of settings commands,
// $..= and [..], with and without a value.
EError SettingsManager::ExecuteCommandOrSetting(const char *key, char *value)
{
    // If value is NULL, it means that there was no value string, i.e.
    // $key without =, or [key] with nothing following.
    // If value is not NULL, but the string is empty, that is the form
    // $key= with nothing following the = .  It is important to distinguish
    // those cases so that you can say "$N0=" to clear a startup line.

    // First search the settings list by text name.  If found, set a new
    // value if one is given, otherwise display the current value.
    for (Setting *s = Setting::List; s; s = s->next())
    {
        if (strcasecmp(s->getName(), key) == 0)
        {
            if (value)
            {
                return s->setStringValue(value);
            }
            else
            {
                ShowSetting(s->getName(), s->getStringValue(), NULL);
                return EError::Ok;
            }
        }
    }

    // Then search the setting list by compatible name.  If found, set a new
    // value if one is given, otherwise display the current value in compatible mode.
    for (Setting *s = Setting::List; s; s = s->next())
    {
        if (s->getGrblName() && strcasecmp(s->getGrblName(), key) == 0)
        {
            if (value)
            {
                return s->setStringValue(value);
            }
            else
            {
                ShowSetting(s->getGrblName(), s->getCompatibleValue(), NULL);
                return EError::Ok;
            }
        }
    }

    // If we did not find a setting, look for a command.  Commands
    // handle values internally; you cannot determine whether to set
    // or display solely based on the presence of a value.
    for (GrblCommand *cp : GRBLCommandsManager::getGRBLCommandsList())
    {
        // if ((strcasecmp(cp->getName(), key) == 0) || (cp->getGrblName() && strcasecmp(cp->getGrblName(), key) == 0))
        if (cp->getName() && strcasecmp(cp->getName(), key) == 0)
        {
            return cp->action(value);
        }
    }

    return EError::InvalidStatement;
}

extern void make_settings();

void SettingsManager::MakeSettings()
{
    make_settings();
}

void SettingsManager::LoadSettings()
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        s->load();
    }
}

void SettingsManager::RestoreSettings(uint8_t restore_flag)
{
    if (restore_flag & SettingsRestore::Defaults)
    {
        bool restore_startup = restore_flag & SettingsRestore::StartupLines;
        for (Setting *s = Setting::List; s; s = s->next())
        {
            if (!s->getDescription())
            {
                const char *name = s->getName();
                if (restore_startup)
                { // all settings get restored
                    s->setDefault();
                }
                else if ((strcmp(name, "Line0") != 0) && (strcmp(name, "Line1") != 0))
                { // non startup settings get restored
                    s->setDefault();
                }
            }
        }
        MessageSender::SendMessage(EMessageLevel::Info, "Settings reset done");
    }

    if (restore_flag & SettingsRestore::Parameters)
    {
        CoordinatesManager::Reset();
    }
}

EError SettingsManager::ExecuteLine(char *line)
{
    char *value;

    // Skip the leading '$' character
    if (*line == '$')
    {
        line++;
    }

    // Find the '=' separator between key and value
    value = strchr(line, '=');
    if (value)
    {
        // $xxx=yyy form: split the string into key and value
        *value++ = '\0';
    }

    char *key = NormalizeKey(line);

    // At this point there are three possibilities for value:
    // NULL         - $xxx without =    (display current value)
    // empty string - $xxx=             (set empty value)
    // non-empty    - $xxx=yyy          (set value)
    return ExecuteCommandOrSetting(key, value);
}

void SettingsManager::ExecuteStartupLines(char *line)
{
    EError status_code;
    char gcline[256];
    strncpy(gcline, startup_line_0->get(), 255);
    if (*gcline)
    {
        status_code = gc_execute_line(gcline);
        report_execute_startup_message(gcline, status_code);
    }
    strncpy(gcline, startup_line_1->get(), 255);
    if (*gcline)
    {
        status_code = gc_execute_line(gcline);
        report_execute_startup_message(gcline, status_code);
    }
}

EError SettingsManager::ReportNormalSettings(const char *value)
{
    ShowGrblSettings(ERabbitGRBLItemType::SETTING, false); // GRBL non-axis settings
    ShowGrblSettings(ERabbitGRBLItemType::SETTING, true);  // GRBL axis settings
    return EError::Ok;
}

EError SettingsManager::ReportExtendedSettings(const char *value)
{
    ShowGrblSettings(ERabbitGRBLItemType::EXTENDED_SETTING, false); // Extended non-axis settings
    ShowGrblSettings(ERabbitGRBLItemType::EXTENDED_SETTING, true);  // Extended axis settings
    return EError::Ok;
}

EError SettingsManager::ListGrblNames(const char *value)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        const char *gn = s->getGrblName();
        if (gn)
        {
            ConnectionManager::WriteFormatted("$%s => $%s\r\n", gn, s->getName());
        }
    }
    return EError::Ok;
}

EError SettingsManager::ListSettings(const char *value)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        const char *displayValue = s->getStringValue();
        ShowSetting(s->getName(), displayValue, NULL);
    }
    return EError::Ok;
}

EError SettingsManager::ListChangedSettings(const char *value)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        const char *value = s->getStringValue();
        const char *defval = s->getDefaultString();
        if (strcmp(value, defval))
        {
            String message = "(Default=";
            message += defval;
            message += ")";
            ShowSetting(s->getName(), value, message.c_str());
        }
    }

    return EError::Ok;
}

EError SettingsManager::RestoreSettingsCommand(const char *value)
{
    if (!value)
    {
        return EError::InvalidStatement;
    }
    auto it = fRestoreCommands.find(value);
    if (it == fRestoreCommands.end())
    {
        return EError::InvalidStatement;
    }
    RestoreSettings(it->second);
    return EError::Ok;
}
