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
#include "../Regex.h"

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
        {"@", SettingsRestore::Wifi},
        {"wifi", SettingsRestore::Wifi},
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

    ConnectionManager::Active().Write(setting);
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
    char c;

    // In the usual case, this loop will exit on the very first test,
    // because the first character is likely to be non-white.
    // Null ('\0') is not considered to be a space character.
    while (isspace(c = *start) && c != '\0')
    {
        ++start;
    }

    // start now points to either a printable character or end of string
    if (c == '\0')
    {
        return start;
    }

    // Having found the beginning of the printable string,
    // we now scan forward until we find a space character.
    char *end;
    for (end = start; (c = *end) != '\0' && !isspace(c); end++)
    {
    }

    // end now points to either a whitespace character or end of string.
    // In either case it is okay to place a null there.
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
    for (Command *cp = Command::List; cp; cp = cp->next())
    {
        if ((strcasecmp(cp->getName(), key) == 0) || (cp->getGrblName() && strcasecmp(cp->getGrblName(), key) == 0))
        {
            return cp->action(value);
        }
    }

    // If we did not find an exact match and there is no value,
    // indicating a display operation, we allow partial matches
    // and display every possibility.  This only applies to the
    // text form of the name, not to the nnn and ESPnnn forms.
    if (!value)
    {
        auto lcKey = String(key);
        lcKey.toLowerCase();
        bool found = false;
        for (Setting *s = Setting::List; s; s = s->next())
        {
            auto lcTest = String(s->getName());
            lcTest.toLowerCase();

            if (regexMatch(lcKey.c_str(), lcTest.c_str()))
            {
                const char *displayValue = s->getStringValue();
                ShowSetting(s->getName(), displayValue, NULL);
                found = true;
            }
        }
        if (found)
        {
            return EError::Ok;
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
        for (auto idx = CoordIndex::Begin; idx < CoordIndex::End; ++idx)
        {
            coords[idx]->setDefault();
        }
    }
    MessageSender::SendMessage(EMessageLevel::Info, "Position offsets reset done");
}

EError SettingsManager::ExecuteLine(char *line)
{
    char *value;
    if (*line++ == '[')
    { // [ESPxxx] form
        value = strrchr(line, ']');
        if (!value)
        {
            // Missing ] is an error in this form
            return EError::InvalidStatement;
        }
        // ']' was found; replace it with null and set value to the rest of the line.
        *value++ = '\0';
        // If the rest of the line is empty, replace value with NULL.
        if (*value == '\0')
        {
            value = NULL;
        }
    }
    else
    {
        // $xxx form
        value = strchr(line, '=');
        if (value)
        {
            // $xxx=yyy form.
            *value++ = '\0';
        }
    }

    char *key = NormalizeKey(line);

    // At this point there are three possibilities for value
    // NULL - $xxx without =
    // NULL - [ESPxxx] with nothing after ]
    // empty string - $xxx= with nothing after
    // non-empty string - [ESPxxx]yyy or $xxx=yyy
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
    ShowGrblSettings(ERabbitGRBLItemType::GRBL, false); // GRBL non-axis settings
    ShowGrblSettings(ERabbitGRBLItemType::GRBL, true);  // GRBL axis settings
    return EError::Ok;
}

EError SettingsManager::ReportExtendedSettings(const char *value)
{
    ShowGrblSettings(ERabbitGRBLItemType::GRBL, false);     // GRBL non-axis settings
    ShowGrblSettings(ERabbitGRBLItemType::EXTENDED, false); // Extended non-axis settings
    ShowGrblSettings(ERabbitGRBLItemType::GRBL, true);      // GRBL axis settings
    ShowGrblSettings(ERabbitGRBLItemType::EXTENDED, true);  // Extended axis settings
    return EError::Ok;
}

EError SettingsManager::ListGrblNames(const char *value)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        const char *gn = s->getGrblName();
        if (gn)
        {
            ConnectionManager::Active().WriteFormatted("$%s => $%s\r\n", gn, s->getName());
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
