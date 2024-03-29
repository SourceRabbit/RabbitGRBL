#pragma once

/*
  Report.h - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

  2018 -	Bart Dring This file was modifed for use on the ESP32
          CPU. Do not use this with Grbl for atMega328P

  2024 - Nikos Siatras (https://github.com/nsiatras)

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

// Define status reporting boolean enable bit flags in status_report_mask
enum RtStatus
{
  Position = bit(0),
  Buffer = bit(1),
};

const char *errorString(Error errorNumber);

// Define Grbl feedback message codes. Valid values (0-255).
enum class Message : uint8_t
{
  CriticalEvent = 1,
  AlarmLock = 2,
  AlarmUnlock = 3,
  Enabled = 4,
  Disabled = 5,
  SafetyDoorAjar = 6,
  CheckLimits = 7,
  ProgramEnd = 8,
  RestoreDefaults = 9,
  SpindleRestore = 10,
  SleepMode = 11,
  SdFileQuit = 60, // mc_reset was called during an SD job
};

enum class MsgLevel : int8_t
{ // Use $Message/Level
  None = 0,
  Error = 1,
  Warning = 2,
  Info = 3,
  Debug = 4,
  Verbose = 5,
};

// functions to send data to the user.
void grbl_sendf(const char *format, ...);
void grbl_msg_sendf(MsgLevel level, const char *format, ...);

// Prints system status messages.
void report_status_message(Error status_code);
void report_realtime_steps();

// Prints system alarm messages.
void report_alarm_message(ExecAlarm alarm_code);

// Prints miscellaneous feedback messages.
void report_feedback_message(Message message);

// Prints welcome message
void report_init_message();

// Prints Grbl help and current global settings
void report_grbl_help();

// Prints an echo of the pre-parsed line received right before execution.
void report_echo_line_received(char *line);

// calculate the postion for status reports
void report_calc_status_position(float *print_position, float *wco, bool wpos);

// Prints realtime status report
void report_realtime_status();

// Prints recorded probe position
void report_probe_parameters();

// Prints Grbl NGC parameters (coordinate offsets, probe)
void report_ngc_parameters();

// Prints current g-code parser mode state
void report_gcode_modes();

// Prints startup line when requested and executed.
void report_startup_line(uint8_t n, const char *line);
void report_execute_startup_message(const char *line, Error status_code);

// Prints build info and user info
void report_build_info(const char *line);

void report_gcode_comment(char *comment);

char report_get_axis_letter(uint8_t axis);
char *reportAxisLimitsMsg(uint8_t axis);
char *reportAxisNameMsg(uint8_t axis);
char *reportAxisNameMsg(uint8_t axis, uint8_t dual_axis);

char *report_state_text();
float *get_wco();
void calc_mpos(float *print_position);
void calc_wpos(float *print_position);
