#pragma once

/*
  Grbl.h - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

  2018 -	Bart Dring This file was modifed for use on the ESP32
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

// Grbl versioning system
const char *const GRBL_VERSION = "1.3a (Rabbit GRBL for the ESP32)";
const char *const GRBL_VERSION_BUILD = "20240303";

#include <Arduino.h>
#include <EEPROM.h>
#include <driver/rmt.h>
#include <Preferences.h>
#include <driver/timer.h>

// Define the Grbl system include files. NOTE: Do not alter organization.
#include "Config.h"
#include "NutsBolts.h"
#include "Defaults.h"
#include "Error.h"
#include "Probe/Probe.h"
#include "System.h"
#include "GCode.h"
#include "Planner.h"
#include "Limits.h"
#include "Backlash.h"
#include "MotionControl.h"
#include "Protocol.h"
#include "Serial.h"
#include "Report.h"
#include "Pins.h"
#include "Spindles/Spindle.h"
#include "Motors/Motors.h"
#include "Stepper.h"
#include "Jog.h"
#include "InputBuffer/InputBuffer.h"
#include "Settings.h"
#include "SettingsDefinitions.h"
#include "Coolant/CoolantManager.h"
#include "UserOutput.h"
#include <Wire.h>

void grbl_init();
void run_once();

void machine_init(); // weak definition in Grbl.cpp
void display_init(); // weak definition in Grbl.cpp

bool user_defined_homing(uint8_t cycle_mask); // weak definition in Limits.cpp

// Called if MACRO_BUTTON_0_PIN or MACRO_BUTTON_1_PIN or MACRO_BUTTON_2_PIN is defined
void user_defined_macro(uint8_t index);

// Called if USE_M30 is defined
void user_m30();

// Called if USE_TOOL_CHANGE is defined
void user_tool_change(uint8_t new_tool);
