#include "Grbl.h"
#include <map>
#include "Regex.h"

void show_setting(const char *name, const char *value, const char *description)
{
    grbl_sendf("$%s=%s", name, value);
    if (description)
    {
        grbl_sendf("    %s", description);
    }
    grbl_sendf("\r\n");
}

void settings_restore(uint8_t restore_flag)
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

// Get settings values from non volatile storage into memory
void load_settings()
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        s->load();
    }
}

extern void make_settings();
extern void make_grbl_commands();

void settings_init()
{
    make_settings();
    make_grbl_commands();
    load_settings();
}

EError show_grbl_help(const char *value)
{
    report_grbl_help();
    return EError::Ok;
}

EError report_gcode(const char *value)
{
    report_gcode_modes();
    return EError::Ok;
}

EError show_grbl_build_info(const char *value)
{
    report_build_info();
    return EError::Ok;
}

void show_grbl_settings(type_t type, bool wantAxis)
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
                show_setting(s->getGrblName(), s->getCompatibleValue(), NULL);
            }
        }
    }
}

EError report_normal_settings(const char *value)
{
    show_grbl_settings(GRBL, false); // GRBL non-axis settings
    show_grbl_settings(GRBL, true);  // GRBL axis settings
    return EError::Ok;
}

EError report_extended_settings(const char *value)
{
    show_grbl_settings(GRBL, false);     // GRBL non-axis settings
    show_grbl_settings(EXTENDED, false); // Extended non-axis settings
    show_grbl_settings(GRBL, true);      // GRBL axis settings
    show_grbl_settings(EXTENDED, true);  // Extended axis settings
    return EError::Ok;
}

EError list_grbl_names(const char *value)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        const char *gn = s->getGrblName();
        if (gn)
        {
            grbl_sendf("$%s => $%s\r\n", gn, s->getName());
        }
    }
    return EError::Ok;
}

EError list_settings(const char *value)
{
    for (Setting *s = Setting::List; s; s = s->next())
    {
        const char *displayValue = s->getStringValue();
        show_setting(s->getName(), displayValue, NULL);
    }
    return EError::Ok;
}

EError list_changed_settings(const char *value)
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
            show_setting(s->getName(), value, message.c_str());
        }
    }

    return EError::Ok;
}

EError list_commands(const char *value)
{
    for (Command *cp = Command::List; cp; cp = cp->next())
    {
        const char *name = cp->getName();
        const char *oldName = cp->getGrblName();
        if (oldName)
        {
            grbl_sendf("$%s or $%s", name, oldName);
        }
        else
        {
            grbl_sendf("$%s", name);
        }
        const char *description = cp->getDescription();
        if (description)
        {
            grbl_sendf(" =%s", description);
        }
        grbl_sendf("\r\n");
    }
    return EError::Ok;
}

EError toggle_check_mode(const char *value)
{
    // Perform reset when toggling off. Check g-code mode should only work if Grbl
    // is idle and ready, regardless of alarm locks. This is mainly to keep things
    // simple and consistent.
    if (sys.state == State::CheckMode)
    {
        mc_reset();
        // MessageSender::SendFeedbackMessage(EFeedbackMessage::Disabled);
    }
    else
    {
        if (sys.state != State::Idle)
        {
            return EError::IdleError; // Requires no alarm mode.
        }
        sys.state = State::CheckMode;
        // MessageSender::SendFeedbackMessage(EFeedbackMessage::Enabled);
    }
    return EError::Ok;
}

EError disable_alarm_lock(const char *value)
{
    if (sys.state == State::Alarm)
    {
        // Block if safety door is ajar.
        if (system_check_safety_door_ajar())
        {
            return EError::CheckDoor;
        }
        MessageSender::SendFeedbackMessage(EFeedbackMessage::AlarmUnlock);
        sys.state = State::Idle;
        // Don't run startup script. Prevents stored moves in startup from causing accidents.
    } // Otherwise, no effect.
    return EError::Ok;
}

EError report_ngc(const char *value)
{
    report_ngc_parameters();
    return EError::Ok;
}

EError home(int cycle)
{
    if (homing_enable->get() == false)
    {
        return EError::SettingDisabled;
    }

    if (system_check_safety_door_ajar())
    {
        return EError::CheckDoor; // Block if safety door is ajar.
    }
    sys.state = State::Homing; // Set system state variable

    mc_homing_cycle(cycle);

    if (!sys.abort)
    {                            // Execute startup scripts after successful homing.
        sys.state = State::Idle; // Set to IDLE when complete.
        st_go_idle();            // Set steppers to the settings idle state before returning.
        if (cycle == HOMING_CYCLE_ALL)
        {
            char line[128];
            system_execute_startup(line);
        }
    }
    return EError::Ok;
}

EError home_all(const char *value)
{
    return home(HOMING_CYCLE_ALL);
}

EError home_x(const char *value)
{
    return home(bit(X_AXIS));
}

EError home_y(const char *value)
{
    return home(bit(Y_AXIS));
}

EError home_z(const char *value)
{
    return home(bit(Z_AXIS));
}

EError home_a(const char *value)
{
    return home(bit(A_AXIS));
}

EError home_b(const char *value)
{
    return home(bit(B_AXIS));
}

EError home_c(const char *value)
{
    return home(bit(C_AXIS));
}

EError sleep_grbl(const char *value)
{
    sys_rt_exec_state.bit.sleep = true;
    return EError::Ok;
}

EError report_startup_lines(const char *value)
{
    report_startup_line(0, startup_line_0->get());
    report_startup_line(1, startup_line_1->get());
    return EError::Ok;
}

std::map<const char *, uint8_t, cmp_str> restoreCommands = {
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

EError restore_settings(const char *value)
{
    if (!value)
    {
        return EError::InvalidStatement;
    }
    auto it = restoreCommands.find(value);
    if (it == restoreCommands.end())
    {
        return EError::InvalidStatement;
    }
    settings_restore(it->second);
    return EError::Ok;
}

EError showState(const char *value)
{
    grbl_sendf("State 0x%x\r\n", sys.state);
    return EError::Ok;
}

EError doJog(const char *value)
{
    // For jogging, you must give gc_execute_line() a line that
    // begins with $J=.  There are several ways we can get here,
    // including  $J, $J=xxx, [J]xxx.  For any form other than
    // $J without =, we reconstruct a $J= line for gc_execute_line().
    if (!value)
    {
        return EError::InvalidStatement;
    }
    char jogLine[LINE_BUFFER_SIZE];
    strcpy(jogLine, "$J=");
    strcat(jogLine, value);
    return gc_execute_line(jogLine);
}

EError motor_disable(const char *value)
{
    char *s;
    if (value == NULL)
    {
        value = "\0";
    }

    s = strdup(value);
    s = trim(s);

    int32_t convertedValue;
    char *endptr;
    if (*s == '\0')
    {
        convertedValue = 255; // all axes
    }
    else
    {
        convertedValue = strtol(s, &endptr, 10);
        if (endptr == s || *endptr != '\0')
        {
            // Try to convert as an axis list
            convertedValue = 0;
            auto axisNames = String("XYZABC");
            while (*s)
            {
                int index = axisNames.indexOf(toupper(*s++));
                if (index < 0)
                {
                    return EError::BadNumberFormat;
                }
                convertedValue |= bit(index);
            }
        }
    }
    CNCMachine::fMotorsManager.SetDisable(true, convertedValue);
    return EError::Ok;
}

// Commands use the same syntax as Settings, but instead of setting or
// displaying a persistent value, a command causes some action to occur.
// That action could be anything, from displaying a run-time parameter
// to performing some system state change.  Each command is responsible
// for decoding its own value string, if it needs one.
void make_grbl_commands()
{
    new GrblCommand("", "Help", show_grbl_help, anyState);
    new GrblCommand("I", "BuildInfo", show_grbl_build_info, idleOrAlarm);
    new GrblCommand("T", "State", showState, anyState);
    new GrblCommand("J", "Jog", doJog, idleOrJog);

    new GrblCommand("$", "GrblSettings/List", report_normal_settings, notCycleOrHold);
    new GrblCommand("+", "ExtendedSettings/List", report_extended_settings, notCycleOrHold);
    new GrblCommand("L", "GrblNames/List", list_grbl_names, notCycleOrHold);
    new GrblCommand("S", "Settings/List", list_settings, notCycleOrHold);
    new GrblCommand("SC", "Settings/ListChanged", list_changed_settings, notCycleOrHold);
    new GrblCommand("CMD", "Commands/List", list_commands, notCycleOrHold);

    new GrblCommand("A", "Alarms/List", [](const char *value) { return CNCMachine::fAlarmsManager.ListAlarms(value); }, anyState);
    new GrblCommand("E", "Errors/List", [](const char *value) { return CNCMachine::fErrorsManager.ListErrors(value); }, anyState);

    new GrblCommand("G", "GCode/Modes", report_gcode, anyState);
    new GrblCommand("C", "GCode/Check", toggle_check_mode, anyState);
    new GrblCommand("X", "Alarm/Disable", disable_alarm_lock, anyState);
    new GrblCommand("NVX", "Settings/Erase", Setting::eraseNVS, idleOrAlarm, WA);
    new GrblCommand("V", "Settings/Stats", Setting::report_nvs_stats, idleOrAlarm);
    new GrblCommand("#", "GCode/Offsets", report_ngc, idleOrAlarm);
    new GrblCommand("H", "Home", home_all, idleOrAlarm);
    new GrblCommand("MD", "Motor/Disable", motor_disable, idleOrAlarm);

#ifdef HOMING_SINGLE_AXIS_COMMANDS
    new GrblCommand("HX", "Home/X", home_x, idleOrAlarm);
    new GrblCommand("HY", "Home/Y", home_y, idleOrAlarm);
    new GrblCommand("HZ", "Home/Z", home_z, idleOrAlarm);
    new GrblCommand("HA", "Home/A", home_a, idleOrAlarm);
    new GrblCommand("HB", "Home/B", home_b, idleOrAlarm);
    new GrblCommand("HC", "Home/C", home_c, idleOrAlarm);
#endif
    new GrblCommand("SLP", "System/Sleep", sleep_grbl, idleOrAlarm);
    new GrblCommand("N", "GCode/StartupLines", report_startup_lines, idleOrAlarm);
    new GrblCommand("RST", "Settings/Restore", restore_settings, idleOrAlarm, WA);
};

// normalize_key puts a key string into canonical form -
// without whitespace.
// start points to a null-terminated string.
// Returns the first substring that does not contain whitespace.
// Case is unchanged because comparisons are case-insensitive.
char *normalize_key(char *start)
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

    // end now points to either a whitespace character of end of string
    // In either case it is okay to place a null there
    *end = '\0';

    return start;
}

// This is the handler for all forms of settings commands,
// $..= and [..], with and without a value.
EError do_command_or_setting(const char *key, char *value)
{
    // If value is NULL, it means that there was no value string, i.e.
    // $key without =, or [key] with nothing following.
    // If value is not NULL, but the string is empty, that is the form
    // $key= with nothing following the = .  It is important to distinguish
    // those cases so that you can say "$N0=" to clear a startup line.

    // First search the settings list by text name.  If found, set a new
    // value if one is given, otherwise display the current value
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
                show_setting(s->getName(), s->getStringValue(), NULL);
                return EError::Ok;
            }
        }
    }

    // Then search the setting list by compatible name.  If found, set a new
    // value if one is given, otherwise display the current value in compatible mode
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
                show_setting(s->getGrblName(), s->getCompatibleValue(), NULL);
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
    EError retval = EError::InvalidStatement;
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
                show_setting(s->getName(), displayValue, NULL);
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

EError system_execute_line(char *line)
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

    char *key = normalize_key(line);

    // At this point there are three possibilities for value
    // NULL - $xxx without =
    // NULL - [ESPxxx] with nothing after ]
    // empty string - $xxx= with nothing after
    // non-empty string - [ESPxxx]yyy or $xxx=yyy
    return do_command_or_setting(key, value);
}

void system_execute_startup(char *line)
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
