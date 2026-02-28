/*
  GRBLCommandsManager.cpp

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

#include "GRBLCommandsManager.h"

// Static definition of the list of all registered GrblCommand objects
std::vector<GrblCommand *> GRBLCommandsManager::fList;

// Initialize and register all GRBL '$' Commands
// https://github.com/SourceRabbit/RabbitGRBL/wiki/GRBL-'$'-Commands
void GRBLCommandsManager::Initialize()
{
    new GrblCommand("", "Help", GRBLCommandsManager::ShowGRBLHelp, grbl_state_any);
    new GrblCommand("I", "Build Info", GRBLCommandsManager::ShowGRBLBuildInfo, grbl_state_idleOrAlarm);
    new GrblCommand("J", "Jog Command", GRBLCommandsManager::DoJog, grbl_state_idleOrJog);

    new GrblCommand("$", "Report Settings", SettingsManager::ReportNormalSettings, grbl_state_notCycleOrHold);
    new GrblCommand("+", "Report Extended Settings", SettingsManager::ReportExtendedSettings, grbl_state_notCycleOrHold);
    new GrblCommand("L", "List Grbl Names", SettingsManager::ListGrblNames, grbl_state_notCycleOrHold);
    new GrblCommand("S", "List Settings", SettingsManager::ListSettings, grbl_state_notCycleOrHold);
    new GrblCommand("SC", "List Changed Settings", SettingsManager::ListChangedSettings, grbl_state_notCycleOrHold);

    new GrblCommand("CMD", "Commands List", GRBLCommandsManager::ListCommands, grbl_state_notCycleOrHold);

    new GrblCommand("A", "Alarms List", AlarmsManager::ListAlarms, grbl_state_any);
    new GrblCommand("E", "Errors List", ErrorsManager::ListErrors, grbl_state_any);

    new GrblCommand("G", "GCode Modes", GRBLCommandsManager::Execute_ReportGCodeModes, grbl_state_any);
    new GrblCommand("C", "Toggle Check Mode", GRBLCommandsManager::ToggleCheckMode, grbl_state_any);
    new GrblCommand("X", "Disable Alarm Lock", GRBLCommandsManager::DisableAlarmLock, grbl_state_any);
    new GrblCommand("NVX", "Erase NVS", NVSManager::EraseNVSUponUserCommand, grbl_state_idleOrAlarm);
    new GrblCommand("V", "Report NVS Stats", GRBLCommandsManager::ReportNVSStats, grbl_state_idleOrAlarm);
    new GrblCommand("#", "Report NGC Parameters", GRBLCommandsManager::Execute_ReportNGCParameters, grbl_state_idleOrAlarm);

    new GrblCommand("H", "Home All Axis", GRBLCommandsManager::HomeAll, grbl_state_idleOrAlarm);

#ifdef HOMING_SINGLE_AXIS_COMMANDS
    // Each axis gets its own $H<axis> command, all routing through Home() with the correct bitmask
    new GrblCommand("HX", "Home X Axis", [](const char *v)
                    { return GRBLCommandsManager::Home(bit(X_AXIS)); }, grbl_state_idleOrAlarm);
    new GrblCommand("HY", "Home Y Axis", [](const char *v)
                    { return GRBLCommandsManager::Home(bit(Y_AXIS)); }, grbl_state_idleOrAlarm);
    new GrblCommand("HZ", "Home Z Axis", [](const char *v)
                    { return GRBLCommandsManager::Home(bit(Z_AXIS)); }, grbl_state_idleOrAlarm);
    new GrblCommand("HA", "Home A Axis", [](const char *v)
                    { return GRBLCommandsManager::Home(bit(A_AXIS)); }, grbl_state_idleOrAlarm);
    new GrblCommand("HB", "HomeB Axis", [](const char *v)
                    { return GRBLCommandsManager::Home(bit(B_AXIS)); }, grbl_state_idleOrAlarm);
    new GrblCommand("HC", "Home C Axis", [](const char *v)
                    { return GRBLCommandsManager::Home(bit(C_AXIS)); }, grbl_state_idleOrAlarm);
#endif

    new GrblCommand("SLP", "Sleep", GRBLCommandsManager::SystemSleep, grbl_state_idleOrAlarm);
    new GrblCommand("N", "Report Startup Lines", GRBLCommandsManager::ReportStartupLines, grbl_state_idleOrAlarm);
    new GrblCommand("RST", "Restore Settings", SettingsManager::RestoreSettingsCommand, grbl_state_idleOrAlarm);
}

EError GRBLCommandsManager::ShowGRBLHelp(const char *value)
{
    report_grbl_help();
    return EError::Ok;
}

EError GRBLCommandsManager::ShowGRBLBuildInfo(const char *value)
{
    report_build_info();
    return EError::Ok;
}

EError GRBLCommandsManager::DoJog(const char *value)
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

EError GRBLCommandsManager::ToggleCheckMode(const char *value)
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

EError GRBLCommandsManager::ListCommands(const char *value)
{
    for (GrblCommand *cp : GRBLCommandsManager::fList)
    {
        // Build the full command line into a local buffer before sending.
        // This avoids multiple fragmented Write calls per line,
        // which is critical for Bluetooth compatibility.
        char line[RX_BUFFER_SIZE];

        const char *name = cp->getGrblName();
        const char *description = cp->getDescription();

        // Format: "$name = description\r\n"
        if (description)
        {
            snprintf(line, sizeof(line), "$%s = %s\r\n", name, description);
        }
        else
        {
            snprintf(line, sizeof(line), "$%s\r\n", name);
        }

        // Single Write per line — safe for Serial and Bluetooth
        ConnectionManager::Active().Write(line);
    }

    delay_ms(100);
    return EError::Ok;
}

EError GRBLCommandsManager::Home(int cycle)
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
    {
        // Execute startup scripts after successful homing.
        sys.state = State::Idle; // Set to IDLE when complete.
        st_go_idle();            // Set steppers to the settings idle state before returning.
        if (cycle == HOMING_CYCLE_ALL)
        {
            char line[128];
            SettingsManager::ExecuteStartupLines(line);
        }
    }

    return EError::Ok;
}

// Home all axes
EError GRBLCommandsManager::HomeAll(const char *value)
{
    return Home(HOMING_CYCLE_ALL);
}

EError GRBLCommandsManager::Execute_ReportGCodeModes(const char *value)
{
    report_gcode_modes();
    return EError::Ok;
}

EError GRBLCommandsManager::DisableAlarmLock(const char *value)
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

EError GRBLCommandsManager::Execute_ReportNGCParameters(const char *value)
{
    report_ngc_parameters();
    return EError::Ok;
}

/**
 * Reports a message with the NVS Usage
 */
EError GRBLCommandsManager::ReportNVSStats(const char *value)
{
    nvs_stats_t stats;

    // Retrieve NVS partition usage statistics via NVSManager
    if (NVSManager::GetStats(&stats) != ESP_OK)
    {
        return EError::NvsGetStatsFailed;
    }

    MessageSender::SendMessage(EMessageLevel::Info, "NVS Used: %d Free: %d Total: %d", stats.used_entries, stats.free_entries, stats.total_entries);

    return EError::Ok;
}

EError GRBLCommandsManager::ReportStartupLines(const char *value)
{
    report_startup_line(0, startup_line_0->get());
    report_startup_line(1, startup_line_1->get());
    return EError::Ok;
}

EError GRBLCommandsManager::SystemSleep(const char *value)
{
    sys_rt_exec_state.bit.sleep = true;
    return EError::Ok;
}
