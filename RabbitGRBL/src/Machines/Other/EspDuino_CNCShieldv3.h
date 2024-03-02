/*
  EspDuino_CNCShieldv3.h

  Copyright (c) 2024 Nikolaos Siatras
  Twitter: nsiatras
  Website: https://www.sourcerabbit.com

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
  *******************************************************************************************************
    WARNING:
    This config holds the pin assignment for ESPDUINO-32 Boards with Protoneer V3.0 CNC Shields.

    In order to make it work you need to modify the Protoneer V3.0 CNC Shield and remove the end stop resistor R1
    as this cases the ESP32 into an endless loop of resetting which prevents it from booting up.
    Simply removing the resistor fixes this issue.

    Please notice that the Protoneer V3.0 CNC Shield does not have opto-couplers on the input pins
    and it is more than likely that the slightest electromagnetic interference will trigger
    the limit and probe pins.

    Also notice that the Probe pin is mapped but will require a 10kohm external pullup to 3.3V
    in order to properly work.
  *******************************************************************************************************
*/

#define MACHINE_NAME "EspDuino32_with_CNC_Shield_v3"

#ifdef N_AXIS
#undef N_AXIS
#endif
#define N_AXIS 3

#define DEFAULT_STEP_PULSE_MICROSECONDS 10 // $0
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // $1 KEEP MOTORS ALWAYS ON

#define DEFAULT_JUNCTION_DEVIATION 0.025 // $11 mm
#define DEFAULT_ARC_TOLERANCE 0.005      // $12 mm

#define DEFAULT_DIRECTION_INVERT_MASK 4 // $3 Invert X and Y
#define DEFAULT_INVERT_LIMIT_PINS 0     // $5 No
#define DEFAULT_INVERT_PROBE_PIN 0      // $6 No

#define DEFAULT_SOFT_LIMIT_ENABLE 0     // Soft Limits Disabled
#define DEFAULT_HARD_LIMIT_ENABLE 0     // Hard Limits Disabled
#define DEFAULT_HOMING_ENABLE 0         // Homing Disabled
#define DEFAULT_HOMING_DIR_MASK 3       // $23 move positive dir Z, negative X,Y
#define DEFAULT_HOMING_FEED_RATE 500.0  // $24 mm/min
#define DEFAULT_HOMING_SEEK_RATE 1500.0 // $25 mm/min
#define DEFAULT_HOMING_PULLOFF 2.0      // $27

// TRAVEL
#define DEFAULT_X_MAX_TRAVEL 250.0 // $130 X axis Max Travel (mm)
#define DEFAULT_Y_MAX_TRAVEL 250.0 // $131 Y axis Max Travel (mm)
#define DEFAULT_Z_MAX_TRAVEL 60.0  // $132 Z axis Max Travel (mm)

// SPEEDS AND ACCELLERATIONS
#define DEFAULT_X_STEPS_PER_MM 400.0 // $100 steps/mm
#define DEFAULT_X_MAX_RATE 4000.0    // $110 mm/min
#define DEFAULT_X_ACCELERATION 80.0  // $120 mm/sec^2

#define DEFAULT_Y_STEPS_PER_MM 400.0 // $101 steps/mm
#define DEFAULT_Y_MAX_RATE 4000.0    // $111 mm/min
#define DEFAULT_Y_ACCELERATION 80.0  // $121 mm/sec^2

#define DEFAULT_Z_STEPS_PER_MM 400.0 // $102 steps/mm
#define DEFAULT_Z_MAX_RATE 2500.0    // $112 mm/min
#define DEFAULT_Z_ACCELERATION 60.0  // $122 mm/sec^2

#define DEFAULT_A_STEPS_PER_MM 26.666 // $103 steps/mm or steps/degree
#define DEFAULT_A_MAX_RATE 7200.0     // $113 mm/min or degrees/min
#define DEFAULT_A_ACCELERATION 60.0   // $123 steps/sec^2 mm/sec^2

// Protoneer V3.0 CNC Shield Pinout
#define X_STEP_PIN GPIO_NUM_26
#define X_DIRECTION_PIN GPIO_NUM_16

#define Y_STEP_PIN GPIO_NUM_25
#define Y_DIRECTION_PIN GPIO_NUM_27

#define Z_STEP_PIN GPIO_NUM_17
#define Z_DIRECTION_PIN GPIO_NUM_14

#define STEPPERS_DISABLE_PIN GPIO_NUM_12

#define SPINDLE_PWM_PIN GPIO_NUM_19 // Laser PWM is Spindle Enable on Shield

#define COOLANT_FLOOD_PIN GPIO_NUM_34
#define COOLANT_MIST_PIN GPIO_NUM_36

#define X_LIMIT_PIN GPIO_NUM_13
#define Y_LIMIT_PIN GPIO_NUM_5
#define Z_LIMIT_PIN GPIO_NUM_23

#define PROBE_PIN GPIO_NUM_39 // NEEDS 10kohm EXTERNAL PULLUP