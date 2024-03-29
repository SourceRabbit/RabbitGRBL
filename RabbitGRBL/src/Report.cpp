/*
  Report.cpp - reporting and messaging methods
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

    2018 -	Bart Dring This file was modified for use on the ESP32
                    CPU. Do not use this with Grbl for atMega328P

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
  This file functions as the primary feedback interface for Grbl. Any outgoing data, such
  as the protocol status messages, feedback messages, and status reports, are stored here.
  For the most part, these functions primarily are called from Protocol.cpp methods. If a
  different style feedback is desired (i.e. JSON), then a user can change these following
  methods to accommodate their needs.


    ESP32 Notes:

    Major rewrite to fix issues with BlueTooth. As described here there is a
    when you try to send data a single byte at a time using SerialBT.write(...).
    https://github.com/espressif/arduino-esp32/issues/1537

    A solution is to send messages as a string using SerialBT.print(...). Use
    a short delay after each send. Therefore this file needed to be rewritten
    to work that way. AVR Grbl was written to be super efficient to give it
    good performance. This is far less efficient, but the ESP32 can handle it.
    Do not use this version of the file with AVR Grbl.

    ESP32 discussion here ...  https://github.com/bdring/Grbl_Esp32/issues/3


*/

#include "Grbl.h"
#include <map>

#ifdef REPORT_HEAP
EspClass esp;
#endif
const int DEFAULTBUFFERSIZE = 64;

/**
 * This is a formating version of the Serial.write method.
 * It works like printf
 */
void grbl_sendf(const char *format, ...)
{
    char loc_buf[64];
    char *temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = new char[len + 1];
        if (temp == NULL)
        {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    Serial.write(temp);
    va_end(arg);
    if (temp != loc_buf)
    {
        delete[] temp;
    }
}

// Use to send [MSG:xxxx] Type messages. The level allows messages to be easily suppressed
void grbl_msg_sendf(MsgLevel level, const char *format, ...)
{
    if (message_level != NULL)
    {
        // might be null before messages are setup
        if (level > static_cast<MsgLevel>(message_level->get()))
        {
            return;
        }
    }

    char loc_buf[100];
    char *temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf))
    {
        temp = new char[len + 1];
        if (temp == NULL)
        {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    grbl_sendf("[MSG:%s]\r\n", temp);
    va_end(arg);
    if (temp != loc_buf)
    {
        delete[] temp;
    }
}

static const int coordStringLen = 20;
static const int axesStringLen = coordStringLen * MAX_N_AXIS;

// formats axis values into a string and returns that string in rpt
// NOTE: rpt should have at least size: axesStringLen
static void report_util_axis_values(float *axis_value, char *rpt)
{
    uint8_t idx;
    char axisVal[coordStringLen];
    float unit_conv = 1.0;        // unit conversion multiplier..default is mm
    const char *format = "%4.3f"; // Default - report mm to 3 decimal places
    rpt[0] = '\0';
    if (report_inches->get())
    {
        unit_conv = 1.0 / MM_PER_INCH;
        format = "%4.4f"; // Report inches to 4 decimal places
    }
    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++)
    {
        snprintf(axisVal, coordStringLen - 1, format, axis_value[idx] * unit_conv);
        strcat(rpt, axisVal);
        if (idx < (number_axis->get() - 1))
        {
            strcat(rpt, ",");
        }
    }
}

// This version returns the axis values as a String
static String report_util_axis_values(const float *axis_value)
{
    String rpt = "";
    uint8_t idx;
    char axisVal[coordStringLen];
    float unit_conv = 1.0; // unit conversion multiplier..default is mm
    int decimals = 3;      // Default - report mm to 3 decimal places
    if (report_inches->get())
    {
        unit_conv = 1.0 / MM_PER_INCH;
        decimals = 4; // Report inches to 4 decimal places
    }
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

// Handles the primary confirmation protocol response for streaming interfaces and human-feedback.
// For every incoming line, this method responds with an 'ok' for a successful command or an
// 'error:'  to indicate some error event with the line or some critical system error during
// operation. Errors events can originate from the g-code parser, settings module, or asynchronously
// from a critical error, such as a triggered hard limit. Interface should always monitor for these
// responses.
void report_status_message(Error status_code)
{
    switch (status_code)
    {
    case Error::Ok: // Error::Ok
        Serial.write("ok\r\n");
        break;
    default:

        // With verbose errors, the message text is displayed instead of the number.
        // Grbl 0.9 used to display the text, while Grbl 1.1 switched to the number.
        // Many senders support both formats.
        if (verbose_errors->get())
        {
            grbl_sendf("error: %s\r\n", errorString(status_code));
        }
        else
        {
            grbl_sendf("error:%d\r\n", static_cast<int>(status_code));
        }
    }
}

// Prints alarm messages.
void report_alarm_message(ExecAlarm alarm_code)
{
    grbl_sendf("ALARM:%d\r\n", static_cast<int>(alarm_code)); // OK to send to all clients
    delay_ms(500);                                            // Force delay to ensure message clears serial write buffer.
}

std::map<Message, const char *> MessageText = {
    {Message::CriticalEvent, "Reset to continue"},
    {Message::AlarmLock, "'$H'|'$X' to unlock"},
    {Message::AlarmUnlock, "Caution: Unlocked"},
    {Message::Enabled, "Enabled"},
    {Message::Disabled, "Disabled"},
    {Message::SafetyDoorAjar, "Check door"},
    {Message::CheckLimits, "Check limits"},
    {Message::ProgramEnd, "Program End"},
    {Message::RestoreDefaults, "Restoring defaults"},
    {Message::SpindleRestore, "Restoring spindle"},
    {Message::SleepMode, "Sleeping"},
    // Handled separately due to numeric argument
    // { Message::SdFileQuit, "Reset during SD file at line: %d" },
};

// Prints feedback messages. This serves as a centralized method to provide additional
// user feedback for things that are not of the status/alarm message protocol. These are
// messages such as setup warnings, switch toggling, and how to exit alarms.
// NOTE: For interfaces, messages are always placed within brackets. And if silent mode
// is installed, the message number codes are less than zero.
void report_feedback_message(Message message)
{
    // ok to send to all clients
    auto it = MessageText.find(message);
    if (it != MessageText.end())
    {
        grbl_msg_sendf(MsgLevel::Info, it->second);
    }
}

// Welcome message
void report_init_message()
{
    grbl_sendf("\r\nGrbl %s %s ['$' for help]\r\n", GRBL_VERSION, GRBL_VERSION_BUILD);
    // grbl_msg_sendf(MsgLevel::Info, "Compiled with ESP32 SDK:%s", ESP.getSdkVersion());
}

// Grbl help message
void report_grbl_help()
{
    Serial.write("[HLP:$$ $+ $# $S $L $G $I $N $x=val $Nx=line $J=line $SLP $C $X $H $F $E=err ~ ! ? ctrl-x]\r\n");
}

// Prints current probe parameters. Upon a probe command, these parameters are updated upon a
// successful probe or upon a failed probe with the G38.3 without errors command (if supported).
// These values are retained until Grbl is power-cycled, whereby they will be re-zeroed.
void report_probe_parameters()
{
    // Report in terms of machine position.
    float print_position[MAX_N_AXIS];
    char probe_rpt[(axesStringLen + 13 + 6 + 1)]; // the probe report we are building here
    char temp[axesStringLen];
    strcpy(probe_rpt, "[PRB:"); // initialize the string with the first characters
    // get the machine position and put them into a string and append to the probe report
    system_convert_array_steps_to_mpos(print_position, sys_probe_position);
    report_util_axis_values(print_position, temp);
    strcat(probe_rpt, temp);
    // add the success indicator and add closing characters
    sprintf(temp, ":%d]\r\n", sys.probe_succeeded);
    strcat(probe_rpt, temp);
    Serial.write(probe_rpt); // send the report
}

// Prints Grbl NGC parameters (coordinate offsets, probing)
void report_ngc_parameters()
{
    String ngc_rpt = "";

    // Print persistent offsets G54 - G59, G28, and G30
    for (auto coord_select = CoordIndex::Begin; coord_select < CoordIndex::End; ++coord_select)
    {
        ngc_rpt += "[";
        ngc_rpt += coords[coord_select]->getName();
        ngc_rpt += ":";
        ngc_rpt += report_util_axis_values(coords[coord_select]->get());
        ngc_rpt += "]\r\n";
    }
    ngc_rpt += "[G92:"; // Print non-persistent G92,G92.1
    ngc_rpt += report_util_axis_values(gc_state.coord_offset);
    ngc_rpt += "]\r\n";
    ngc_rpt += "[TLO:"; // Print tool length offset
    float tlo = gc_state.tool_length_offset;
    if (report_inches->get())
    {
        tlo *= INCH_PER_MM;
    }
    ngc_rpt += String(tlo, 3);
    ;
    ngc_rpt += "]\r\n";
    Serial.write(ngc_rpt.c_str());
    report_probe_parameters();
}

// Print current gcode parser mode state
void report_gcode_modes()
{
    char temp[20];
    char modes_rpt[75];
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
    }
    strcat(modes_rpt, mode);

    sprintf(temp, " G%d", gc_state.modal.coord_select + 54);
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
    switch (gc_state.modal.arc_distance) {
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
    if (CoolantManager::AreAllCoolantsOff())
    {
        // All coolants are off. Report with M9
        strcat(modes_rpt, " M9");
    }
    else
    {
        // Note: Multiple coolant states may be active at the same time.
        if (CoolantManager::Mist_Coolant.isOn())
        {
            strcat(modes_rpt, " M7");
        }

        if (CoolantManager::Flood_Coolant.isOn())
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
    sprintf(temp, report_inches->get() ? " F%.1f" : " F%.0f", gc_state.feed_rate);
    strcat(modes_rpt, temp);
    sprintf(temp, " S%d", uint32_t(gc_state.spindle_speed));
    strcat(modes_rpt, temp);
    strcat(modes_rpt, "]\r\n");
    Serial.write(modes_rpt);
}

// Prints specified startup line
void report_startup_line(uint8_t n, const char *line)
{
    grbl_sendf("$N%d=%s\r\n", n, line); // OK to send to all
}

void report_execute_startup_message(const char *line, Error status_code)
{
    grbl_sendf(">%s:", line); // OK to send to all
    report_status_message(status_code);
}

// Prints build info line
void report_build_info(const char *line)
{
    grbl_sendf("[VER:%s.%s:%s]\r\n[OPT:", GRBL_VERSION, GRBL_VERSION_BUILD, line);
#ifdef COOLANT_MIST_PIN
    Serial.write("M"); // TODO Need to deal with M8...it could be disabled
#endif
#ifdef PARKING_ENABLE
    Serial.write("P");
#endif
#ifdef HOMING_SINGLE_AXIS_COMMANDS
    Serial.write("H");
#endif
#ifdef LIMITS_TWO_SWITCHES_ON_AXES
    Serial.write("L");
#endif
#ifdef ALLOW_FEED_OVERRIDE_DURING_PROBE_CYCLES
    Serial.write("A");
#endif
#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
    Serial.write("R");
#endif
#ifndef ENABLE_RESTORE_WIPE_ALL // NOTE: Shown when disabled.
    Serial.write("*");
#endif
#ifndef ENABLE_RESTORE_DEFAULT_SETTINGS // NOTE: Shown when disabled.
    Serial.write("$");
#endif
#ifndef ENABLE_RESTORE_CLEAR_PARAMETERS // NOTE: Shown when disabled.
    Serial.write("#");
#endif
#ifndef FORCE_BUFFER_SYNC_DURING_NVS_WRITE // NOTE: Shown when disabled.
    Serial.write("E");
#endif
#ifndef FORCE_BUFFER_SYNC_DURING_WCO_CHANGE // NOTE: Shown when disabled.
    Serial.write("W");
#endif
    // NOTE: Compiled values, like override increments/max/min values, may be added at some point later.
    // These will likely have a comma delimiter to separate them.
    Serial.write("]\r\n");
}

// Prints the character string line Grbl has received from the user, which has been pre-parsed,
// and has been sent into protocol_execute_line() routine to be executed by Grbl.
void report_echo_line_received(char *line)
{
    grbl_sendf("[echo: %s]\r\n", line);
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
            wco[idx] = gc_state.coord_system[idx] + gc_state.coord_offset[idx];

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
    char status[200];
    char temp[MAX_N_AXIS * 20];

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
        int bufsize = DEFAULTBUFFERSIZE;
        bufsize = client_get_rx_buffer_available();
        sprintf(temp, "|Bf:%d,%d", plan_get_block_buffer_available(), bufsize);
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
    if (report_inches->get())
    {
        sprintf(temp, "|FS:%.1f,%d", st_get_realtime_rate() / MM_PER_INCH, sys.spindle_speed);
    }
    else
    {
        sprintf(temp, "|FS:%.0f,%d", st_get_realtime_rate(), sys.spindle_speed);
    }
    strcat(status, temp);
#endif

#ifdef REPORT_FIELD_PIN_STATE

    AxisMask lim_pin_state = limits_get_state();
    ControlPins ctrl_pin_state = system_control_get_state();

    bool isProbeTriggered = Probe::isTriggered();

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
        default:
            sys.report_ovr_counter = (REPORT_OVR_REFRESH_IDLE_COUNT - 1);
            break;
        }

        sprintf(temp, "|Ov:%d,%d,%d", sys.f_override, sys.r_override, sys.spindle_speed_ovr);
        strcat(status, temp);
        SpindleState sp_state = fSpindle->get_state();

        if (sp_state != SpindleState::Disable || CoolantManager::Flood_Coolant.isOn() || CoolantManager::Mist_Coolant.isOn())
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

            if (CoolantManager::Flood_Coolant.isOn())
            {
                strcat(status, "F");
            }

            if (CoolantManager::Mist_Coolant.isOn())
            {
                strcat(status, "M");
            }
        }
    }
#endif

#ifdef REPORT_HEAP
    sprintf(temp, "|Heap:%d", esp.getHeapSize());
    strcat(status, temp);
#endif
    strcat(status, ">\r\n");
    Serial.write(status);
}

void report_realtime_steps()
{
    uint8_t idx;
    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++)
    {
        grbl_sendf("%ld\n", sys_position[idx]); // OK to send to all ... debug stuff
    }
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
        grbl_msg_sendf(MsgLevel::Info, "GCode Comment...%s", msg);
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
                ; // Door ajar
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
        wco[idx] = gc_state.coord_system[idx] + gc_state.coord_offset[idx];
        if (idx == TOOL_LENGTH_OFFSET_AXIS)
        {
            wco[idx] += gc_state.tool_length_offset;
        }
    }
    return wco;
}
