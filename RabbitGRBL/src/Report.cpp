/*
  Report.cpp - reporting and messaging methods
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

    2018 -	Bart Dring This file was modified for use on the ESP32
                    CPU. Do not use this with Grbl for atMega328P

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

/*
    This file functions as the primary feedback interface for Grbl. Any outgoing data, such
    as the protocol status messages, feedback messages, and status reports, are stored here.
    For the most part, these functions are primarily called from Protocol.cpp methods. If a
    different style of feedback is desired (i.e. JSON), a user can change the following
    methods to accommodate their needs.
*/

#include "Grbl.h"
#include <map>

// Welcome message
void report_init_message()
{
    ConnectionManager::WriteFormatted("\r\n%s Build %s  \r\n", FIRMWARE_NAME, GRBL_VERSION_BUILD);
}

// Grbl help message
void report_grbl_help()
{
    // Build the help string dynamically from registered commands.
    ConnectionManager::Write("[HLP:");

    for (GrblCommand *cmd : GRBLCommandsManager::getGRBLCommandsList())
    {
        const char *grblName = cmd->getName();
        if (grblName != nullptr && grblName[0] != '\0')
        {
            ConnectionManager::Write(" $");
            ConnectionManager::Write(grblName);
        }
    }

    // Append realtime tokens that are not part of the Command registry (if applicable).
    ConnectionManager::Write(" ~ ! ? ctrl-x");

    ConnectionManager::Write("]\r\n");
}

/**
 * Formats axis values into a string and returns that string in rpt.
 * NOTE: rpt should have at least size: axesStringLen
 */
void report_util_axis_values(float *axis_value, char *rpt)
{
    uint8_t idx;
    char axisVal[MAX_COORD_STRING_LENGTH];
    float unit_conv = 1.0;        // unit conversion multiplier..default is mm
    const char *format = "%4.3f"; // Default - report mm to 3 decimal places
    rpt[0] = '\0';

    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++)
    {
        snprintf(axisVal, MAX_COORD_STRING_LENGTH - 1, format, axis_value[idx] * unit_conv);
        strcat(rpt, axisVal);
        if (idx < (number_axis->get() - 1))
        {
            strcat(rpt, ",");
        }
    }
}

/**
 * This version returns the axis values as a String
 */
static String report_util_axis_values(const float *axis_value)
{
    String rpt = "";
    uint8_t idx;
    char axisVal[MAX_COORD_STRING_LENGTH];
    float unit_conv = 1.0; // unit conversion multiplier..default is mm
    int decimals = 3;      // Default - report mm to 3 decimal places

    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++)
    {
        rpt += String(axis_value[idx] * unit_conv, decimals);
        if (idx < (number_axis->get() - 1))
        {
            rpt += ",";
        }
    }
    return rpt;
}

/**
 * Handles the primary confirmation protocol response for streaming interfaces and human-feedback.
 * For every incoming line, this method responds with an 'ok' for a successful command or an
 * 'error:'  to indicate some error event with the line or some critical system error during
 * operation. Errors events can originate from the g-code parser, settings module, or asynchronously
 * from a critical error, such as a triggered hard limit. Interface should always monitor for these
 * responses.
 */
void report_status_message(EError status_code)
{
    switch (status_code)
    {

    case EError::Ok: // EError::Ok
        ConnectionManager::Write("ok\r\n");
        break;

    default:
        // Grbl 0.9 reported errors as text, Grbl 1.1 switched to numeric codes.
        // RabbitGRBL follows the Grbl 1.1 standard, so the error number is reported.
        ConnectionManager::WriteFormatted("error:%d\r\n", static_cast<int>(status_code));
    }
}

// Prints Grbl NGC parameters (coordinate offsets, probing)
void report_ngc_parameters()
{
    String ngc_rpt = "";

    // Print persistent and non persistent offsets (G92, G54 - G59, G28, and G30)
    // Example message: [G54:0.000,0.000,0.000]\r\n
    for (auto coord_select = ECoordinateOffset::Begin; coord_select < ECoordinateOffset::End; ++coord_select)
    {
        ngc_rpt += CoordinatesManager::getOffset(coord_select)->toString();
        ngc_rpt += "\r\n";
    }

    ngc_rpt += "[TLO:"; // Print tool length offset
    float tlo = gc_state.tool_length_offset;
    ngc_rpt += String(tlo, 3);
    ngc_rpt += "]\r\n";

    ConnectionManager::Write(ngc_rpt.c_str());
    Controller::getProbe().ReportProbeParameters();

    delay_ms(100);
}

// Print current gcode parser mode state
void report_gcode_modes()
{
    char temp[20];
    char modes_rpt[128];
    const char *mode = "";
    strcpy(modes_rpt, "[GC:");

    switch (gc_state.modal.motion)
    {
    case Motion::None:
        mode = "G80";
        break;
    case Motion::Seek:
        mode = "G0";
        break;
    case Motion::Linear:
        mode = "G1";
        break;
    case Motion::CwArc:
        mode = "G2";
        break;
    case Motion::CcwArc:
        mode = "G3";
        break;
    case Motion::ProbeToward:
        mode = "G38.1";
        break;
    case Motion::ProbeTowardNoError:
        mode = "G38.2";
        break;
    case Motion::ProbeAway:
        mode = "G38.3";
        break;
    case Motion::ProbeAwayNoError:
        mode = "G38.4";
        break;
    case Motion::Drill:
        mode = "G81";
        break;
    case Motion::PeckDrill_ChipRemoving:
        mode = "G83";
        break;
    case Motion::PeckDrill_ChipBreaking:
        mode = "G73";
        break;
    }
    strcat(modes_rpt, mode);

    sprintf(temp, " G%d", static_cast<uint8_t>(gc_state.modal.coord_select) + 54);
    strcat(modes_rpt, temp);

    switch (gc_state.modal.plane_select)
    {
    case Plane::XY:
        mode = " G17";
        break;
    case Plane::ZX:
        mode = " G18";
        break;
    case Plane::YZ:
        mode = " G19";
        break;
    }
    strcat(modes_rpt, mode);

    switch (gc_state.modal.units)
    {
    case Units::Inches:
        mode = " G20";
        break;
    case Units::Mm:
        mode = " G21";
        break;
    }
    strcat(modes_rpt, mode);

    switch (gc_state.modal.distance)
    {
    case Distance::Absolute:
        mode = " G90";
        break;
    case Distance::Incremental:
        mode = " G91";
        break;
    }
    strcat(modes_rpt, mode);

#if 0
    switch (gc_state.modal.arc_distance) 
    {
        case ArcDistance::Absolute: mode = " G90.1"; break;
        case ArcDistance::Incremental: mode = " G91.1"; break;
    }
    strcat(modes_rpt, mode);
#endif

    switch (gc_state.modal.feed_rate)
    {
    case FeedRate::UnitsPerMin:
        mode = " G94";
        break;
    case FeedRate::InverseTime:
        mode = " G93";
        break;
    }
    strcat(modes_rpt, mode);

    // report_util_gcode_modes_M();
    switch (gc_state.modal.program_flow)
    {
    case ProgramFlow::Running:
        mode = "";
        break;
    case ProgramFlow::Paused:
        mode = " M0";
        break;
    case ProgramFlow::OptionalStop:
        mode = " M1";
        break;
    case ProgramFlow::CompletedM2:
        mode = " M2";
        break;
    case ProgramFlow::CompletedM30:
        mode = " M30";
        break;
    }
    strcat(modes_rpt, mode);

    switch (gc_state.modal.spindle)
    {
    case SpindleState::Cw:
        mode = " M3";
        break;
    case SpindleState::Ccw:
        mode = " M4";
        break;
    case SpindleState::Disable:
        mode = " M5";
        break;
    default:
        mode = "";
    }
    strcat(modes_rpt, mode);

    // report_util_gcode_modes_M();  // optional M7 and M8 should have been dealt with by here
    if (Controller::getCoolantManager().AreAllCoolantsOff())
    {
        // All coolants are off. Report with M9
        strcat(modes_rpt, " M9");
    }
    else
    {
        // Note: Multiple coolant states may be active at the same time.
        if (Controller::getCoolantManager().Mist_Coolant.isOn())
        {
            strcat(modes_rpt, " M7");
        }

        if (Controller::getCoolantManager().Flood_Coolant.isOn())
        {
            strcat(modes_rpt, " M8");
        }
    }

#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
    if (sys.override_ctrl == Override::ParkingMotion)
    {
        strcat(modes_rpt, " M56");
    }
#endif

    sprintf(temp, " T%d", gc_state.tool);
    strcat(modes_rpt, temp);
    sprintf(temp, " F%.0f", gc_state.feed_rate);
    strcat(modes_rpt, temp);
    sprintf(temp, " S%d", uint32_t(gc_state.spindle_speed));
    strcat(modes_rpt, temp);
    strcat(modes_rpt, "]\r\n");
    ConnectionManager::Write(modes_rpt);
}

// Prints specified startup line
void report_startup_line(uint8_t n, const char *line)
{
    ConnectionManager::WriteFormatted("$N%d=%s\r\n", n, line); // OK to send to all
}

void report_execute_startup_message(const char *line, EError status_code)
{
    ConnectionManager::WriteFormatted(">%s:", line); // OK to send to all
    report_status_message(status_code);
}

// Prints build info line
void report_build_info()
{
    ConnectionManager::WriteFormatted("[VER:%s.%s]\r\n", GRBL_VERSION, GRBL_VERSION_BUILD);
    ConnectionManager::Write("[OPT:"
#ifdef COOLANT_MIST_PIN
                             "M"
#endif
#ifdef PARKING_ENABLE
                             "P"
#endif
#ifdef HOMING_SINGLE_AXIS_COMMANDS
                             "H"
#endif
#ifdef LIMITS_TWO_SWITCHES_ON_AXES
                             "L"
#endif
#ifdef ALLOW_FEED_OVERRIDE_DURING_PROBE_CYCLES
                             "A"
#endif
#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
                             "R"
#endif
#ifndef ENABLE_RESTORE_WIPE_ALL
                             "*"
#endif
#ifndef ENABLE_RESTORE_DEFAULT_SETTINGS
                             "$"
#endif
#ifndef ENABLE_RESTORE_CLEAR_PARAMETERS
                             "#"
#endif
#ifndef FORCE_BUFFER_SYNC_DURING_NVS_WRITE
                             "E"
#endif
#ifndef FORCE_BUFFER_SYNC_DURING_WCO_CHANGE
                             "W"
#endif
                             "]\r\n");
}

// Prints the character string line Grbl has received from the user, which has been pre-parsed,
// and has been sent into protocol_execute_line() routine to be executed by Grbl.
void report_echo_line_received(char *line)
{
    ConnectionManager::WriteFormatted("[echo: %s]\r\n", line);
}

// Calculate the position for status reports.
// float print_position = returned position
// float wco            = returns the work coordinate offset
// bool wpos            = true for work position compensation
void report_calc_status_position(float *print_position, float *wco, bool wpos)
{
    int32_t current_position[MAX_N_AXIS]; // Copy current state of the system position variable
    memcpy(current_position, sys_position, sizeof(sys_position));
    system_convert_array_steps_to_mpos(print_position, current_position);

    // float wco[MAX_N_AXIS];
    if (wpos || (sys.report_wco_counter == 0))
    {
        auto n_axis = number_axis->get();
        for (uint8_t idx = 0; idx < n_axis; idx++)
        {
            // Apply work coordinate offsets and tool length offset to current position.
            wco[idx] = gc_state.coord_system[idx] + CoordinatesManager::getOffset(ECoordinateOffset::G92)->get()[idx];

            if (idx == TOOL_LENGTH_OFFSET_AXIS)
            {
                wco[idx] += gc_state.tool_length_offset;
            }

            if (wpos)
            {
                print_position[idx] -= wco[idx];
            }
        }
    }
}

// Prints real-time data. This function grabs a real-time snapshot of the stepper subprogram
// and the actual location of the CNC machine. Users may change the following function to their
// specific needs, but the desired real-time data report must be as short as possible. This is
// requires as it minimizes the computational overhead and allows grbl to keep running smoothly,
// especially during g-code programs with fast, short line segments and high frequency reports (5-20Hz).
void report_realtime_status()
{
    float print_position[MAX_N_AXIS];
    char status[256];
    char temp[MAX_AXES_STRING_LENGTH];

    strcpy(status, "<");
    strcat(status, report_state_text());

    // Report position
    if (bit_istrue(status_mask->get(), RtStatus::Position))
    {
        calc_mpos(print_position);
        strcat(status, "|MPos:");
    }
    else
    {
        calc_wpos(print_position);
        strcat(status, "|WPos:");
    }
    report_util_axis_values(print_position, temp);
    strcat(status, temp);
    // Returns planner and serial read buffer states.
#ifdef REPORT_FIELD_BUFFER_STATE
    if (bit_istrue(status_mask->get(), RtStatus::Buffer))
    {
        // Report available planner blocks and RX buffer space.
        sprintf(temp, "|Bf:%d,%d", plan_get_block_buffer_available(), ConnectionManager::GetRxBufferAvailable());
        strcat(status, temp);
    }
#endif
#ifdef USE_LINE_NUMBERS
#ifdef REPORT_FIELD_LINE_NUMBERS
    // Report current line number
    plan_block_t *cur_block = plan_get_current_block();
    if (cur_block != NULL)
    {
        uint32_t ln = cur_block->line_number;
        if (ln > 0)
        {
            sprintf(temp, "|Ln:%d", ln);
            strcat(status, temp);
        }
    }
#endif
#endif
    // Report realtime feed speed
#ifdef REPORT_FIELD_CURRENT_FEED_SPEED
    sprintf(temp, "|FS:%.0f,%d", st_get_realtime_rate(), sys.spindle_speed);
    strcat(status, temp);
#endif

#ifdef REPORT_FIELD_PIN_STATE

    AxisMask lim_pin_state = limits_get_state();
    ControlPins ctrl_pin_state = system_control_get_state();

    bool isProbeTriggered = Controller::getProbe().isTriggered();

    if (lim_pin_state || ctrl_pin_state.value || isProbeTriggered)
    {
        strcat(status, "|Pn:");
        if (isProbeTriggered)
        {
            strcat(status, "P");
        }

        if (lim_pin_state)
        {
            auto n_axis = number_axis->get();
            if (n_axis >= 1 && bit_istrue(lim_pin_state, bit(X_AXIS)))
            {
                strcat(status, "X");
            }
            if (n_axis >= 2 && bit_istrue(lim_pin_state, bit(Y_AXIS)))
            {
                strcat(status, "Y");
            }
            if (n_axis >= 3 && bit_istrue(lim_pin_state, bit(Z_AXIS)))
            {
                strcat(status, "Z");
            }
            if (n_axis >= 4 && bit_istrue(lim_pin_state, bit(A_AXIS)))
            {
                strcat(status, "A");
            }
            if (n_axis >= 5 && bit_istrue(lim_pin_state, bit(B_AXIS)))
            {
                strcat(status, "B");
            }
            if (n_axis >= 6 && bit_istrue(lim_pin_state, bit(C_AXIS)))
            {
                strcat(status, "C");
            }
        }

        if (ctrl_pin_state.value)
        {
            if (ctrl_pin_state.bit.safetyDoor)
            {
                strcat(status, "D");
            }
            if (ctrl_pin_state.bit.reset)
            {
                strcat(status, "R");
            }
            if (ctrl_pin_state.bit.feedHold)
            {
                strcat(status, "H");
            }
            if (ctrl_pin_state.bit.cycleStart)
            {
                strcat(status, "S");
            }
            if (ctrl_pin_state.bit.macro0)
            {
                strcat(status, "0");
            }
            if (ctrl_pin_state.bit.macro1)
            {
                strcat(status, "1");
            }
            if (ctrl_pin_state.bit.macro2)
            {
                strcat(status, "2");
            }
            if (ctrl_pin_state.bit.macro3)
            {
                strcat(status, "3");
            }
        }
    }
#endif
#ifdef REPORT_FIELD_WORK_COORD_OFFSET
    if (sys.report_wco_counter > 0)
    {
        sys.report_wco_counter--;
    }
    else
    {
        switch (sys.state)
        {
        case State::Homing:
        case State::Cycle:
        case State::Hold:
        case State::Jog:
        case State::SafetyDoor:
            sys.report_wco_counter = (REPORT_WCO_REFRESH_BUSY_COUNT - 1); // Reset counter for slow refresh
            break;
        default:
            sys.report_wco_counter = (REPORT_WCO_REFRESH_IDLE_COUNT - 1);
            break;
        }
        if (sys.report_ovr_counter == 0)
        {
            sys.report_ovr_counter = 1; // Set override on next report.
        }
        strcat(status, "|WCO:");
        report_util_axis_values(get_wco(), temp);
        strcat(status, temp);
    }
#endif
#ifdef REPORT_FIELD_OVERRIDES
    if (sys.report_ovr_counter > 0)
    {
        sys.report_ovr_counter--;
    }
    else
    {
        switch (sys.state)
        {
        case State::Homing:
        case State::Cycle:
        case State::Hold:
        case State::Jog:
        case State::SafetyDoor:
            sys.report_ovr_counter = (REPORT_OVR_REFRESH_BUSY_COUNT - 1); // Reset counter for slow refresh
            break;
        default:
            sys.report_ovr_counter = (REPORT_OVR_REFRESH_IDLE_COUNT - 1);
            break;
        }

        sprintf(temp, "|Ov:%d,%d,%d", sys.f_override, sys.r_override, sys.spindle_speed_ovr);
        strcat(status, temp);
        SpindleState sp_state = Controller::getSpindle()->getState();

        if (sp_state != SpindleState::Disable || Controller::getCoolantManager().Flood_Coolant.isOn() || Controller::getCoolantManager().Mist_Coolant.isOn())
        {
            strcat(status, "|A:");
            switch (sp_state)
            {
            case SpindleState::Disable:
                break;
            case SpindleState::Cw:
                strcat(status, "S");
                break;
            case SpindleState::Ccw:
                strcat(status, "C");
                break;
            }

            if (Controller::getCoolantManager().Flood_Coolant.isOn())
            {
                strcat(status, "F");
            }

            if (Controller::getCoolantManager().Mist_Coolant.isOn())
            {
                strcat(status, "M");
            }
        }
    }
#endif

    strcat(status, ">\r\n");
    ConnectionManager::Write(status);
}

void report_gcode_comment(char *comment)
{
    char msg[80];
    const uint8_t offset = 4; // ignore "MSG_" part of comment
    uint8_t index = offset;
    if (strstr(comment, "MSG"))
    {
        while (index < strlen(comment))
        {
            msg[index - offset] = comment[index];
            index++;
        }
        msg[index - offset] = 0; // null terminate
        MessageSender::SendMessage(EMessageLevel::Info, "GCode Comment...%s", msg);
    }
}

char *report_state_text()
{
    static char state[10];

    switch (sys.state)
    {
    case State::Idle:
        strcpy(state, "Idle");
        break;
    case State::Cycle:
        strcpy(state, "Run");
        break;
    case State::Hold:
        if (!(sys.suspend.bit.jogCancel))
        {
            sys.suspend.bit.holdComplete ? strcpy(state, "Hold:0") : strcpy(state, "Hold:1");
            break;
        } // Continues to print jog state during jog cancel.
    case State::Jog:
        strcpy(state, "Jog");
        break;
    case State::Homing:
        strcpy(state, "Home");
        break;
    case State::Alarm:
        strcpy(state, "Alarm");
        break;
    case State::CheckMode:
        strcpy(state, "Check");
        break;
    case State::SafetyDoor:
        strcpy(state, "Door:");
        if (sys.suspend.bit.initiateRestore)
        {
            strcat(state, "3"); // Restoring
        }
        else
        {
            if (sys.suspend.bit.retractComplete)
            {
                sys.suspend.bit.safetyDoorAjar ? strcat(state, "1") : strcat(state, "0");
                // Door ajar
                // Door closed and ready to resume
            }
            else
            {
                strcat(state, "2"); // Retracting
            }
        }
        break;
    case State::Sleep:
        strcpy(state, "Sleep");
        break;
    }
    return state;
}

char report_get_axis_letter(uint8_t axis)
{
    switch (axis)
    {
    case X_AXIS:
        return 'X';
    case Y_AXIS:
        return 'Y';
    case Z_AXIS:
        return 'Z';
    case A_AXIS:
        return 'A';
    case B_AXIS:
        return 'B';
    case C_AXIS:
        return 'C';
    default:
        return '?';
    }
}

char *reportAxisLimitsMsg(uint8_t axis)
{
    static char msg[40];
    sprintf(msg, "Limits(%0.3f,%0.3f)", limitsMinPosition(axis), limitsMaxPosition(axis));
    return msg;
}

char *reportAxisNameMsg(uint8_t axis, uint8_t dual_axis)
{
    static char name[10];
    sprintf(name, "%c%c Axis", report_get_axis_letter(axis), dual_axis ? '2' : ' ');
    return name;
}

char *reportAxisNameMsg(uint8_t axis)
{
    static char name[10];
    sprintf(name, "%c  Axis", report_get_axis_letter(axis));
    return name;
}

void calc_mpos(float *print_position)
{
    int32_t current_position[MAX_N_AXIS]; // Copy current state of the system position variable
    memcpy(current_position, sys_position, sizeof(sys_position));
    system_convert_array_steps_to_mpos(print_position, current_position);
}

void calc_wpos(float *print_position)
{
    int32_t current_position[MAX_N_AXIS]; // Copy current state of the system position variable
    memcpy(current_position, sys_position, sizeof(sys_position));
    system_convert_array_steps_to_mpos(print_position, current_position);

    float *wco = get_wco();
    auto n_axis = number_axis->get();
    for (int idx = 0; idx < n_axis; idx++)
    {
        print_position[idx] -= wco[idx];
    }
}

float *get_wco()
{
    static float wco[MAX_N_AXIS];
    auto n_axis = number_axis->get();
    for (int idx = 0; idx < n_axis; idx++)
    {
        // Apply work coordinate offsets and tool length offset to current position.
        wco[idx] = gc_state.coord_system[idx] + CoordinatesManager::getOffset(ECoordinateOffset::G92)->get()[idx];
        if (idx == TOOL_LENGTH_OFFSET_AXIS)
        {
            wco[idx] += gc_state.tool_length_offset;
        }
    }
    return wco;
}
