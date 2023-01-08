#pragma once

#define MACHINE_NAME    "SourceRabbit 4Axis CNC Motherboard"

#ifdef N_AXIS
	#undef N_AXIS
#endif
#define N_AXIS 4

#define DEFAULT_STEP_PULSE_MICROSECONDS 	10  	// $0
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 		255	// $1 KEEP MOTORS ALWAYS ON

#define DEFAULT_JUNCTION_DEVIATION 		0.025  	// $11 mm
#define DEFAULT_ARC_TOLERANCE 			0.005  	// $12 mm

#define DEFAULT_DIRECTION_INVERT_MASK                   4
#define DEFAULT_INVERT_LIMIT_PINS 			0
#define DEFAULT_INVERT_PROBE_PIN 			0

#define DEFAULT_SOFT_LIMIT_ENABLE 			1
#define DEFAULT_HARD_LIMIT_ENABLE 			0
#define DEFAULT_HOMING_ENABLE 				1
#define DEFAULT_HOMING_DIR_MASK 			3  	// $23 move positive dir Z, negative X,Y
#define DEFAULT_HOMING_FEED_RATE 			500.0  	// $24 mm/min
#define DEFAULT_HOMING_SEEK_RATE 			1500.0  // $25 mm/min
#define DEFAULT_HOMING_PULLOFF 				2.0  	// $27 


// TRAVEL
#define DEFAULT_X_MAX_TRAVEL 				302.0  	// $130 mm 
#define DEFAULT_Y_MAX_TRAVEL 				202.0  	// $131 mm
#define DEFAULT_Z_MAX_TRAVEL 				180.0  	// $132 mm
#define DEFAULT_A_MAX_TRAVEL 				0  		// $133 mm (NOTE: We set this to zero in order to avoid Soft Limits for this Axis)

// SPEEDS AND ACCELLERATIONS
// DRIVERS MICROSTEPPING IS SET to x8
#define DEFAULT_X_STEPS_PER_MM 				400.0
#define DEFAULT_X_MAX_RATE 					4000.0  // mm/min
#define DEFAULT_X_ACCELERATION 				80.0

#define DEFAULT_Y_STEPS_PER_MM 				400.0
#define DEFAULT_Y_MAX_RATE 					4000.0  // mm/min
#define DEFAULT_Y_ACCELERATION 				80.0

#define DEFAULT_Z_STEPS_PER_MM 				400.0
#define DEFAULT_Z_MAX_RATE 					2500.0  // mm/min
#define DEFAULT_Z_ACCELERATION 				60.0

#define DEFAULT_A_STEPS_PER_MM 				26.666	// Steps / Degree
#define DEFAULT_A_MAX_RATE 					7200.0  // degrees/min
#define DEFAULT_A_ACCELERATION 				60.0


// SourceRabbit CNC Motherboard v1.2 Pinout
#define X_STEP_PIN              GPIO_NUM_0
#define X_DIRECTION_PIN         GPIO_NUM_33
#define Y_STEP_PIN              GPIO_NUM_25
#define Y_DIRECTION_PIN         GPIO_NUM_26
#define Z_STEP_PIN              GPIO_NUM_27
#define Z_DIRECTION_PIN         GPIO_NUM_14
#define A_STEP_PIN              GPIO_NUM_12
#define A_DIRECTION_PIN         GPIO_NUM_13
#define STEPPERS_DISABLE_PIN    GPIO_NUM_15

#define SPINDLE_TYPE            SpindleType::PWM 
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_21
#define SPINDLE_DIR_PIN      	GPIO_NUM_2

#define X_LIMIT_PIN             GPIO_NUM_36
#define Y_LIMIT_PIN             GPIO_NUM_39
#define Z_LIMIT_PIN             GPIO_NUM_34
#define PROBE_PIN               GPIO_NUM_32

#define COOLANT_MIST_PIN        GPIO_NUM_22
#define COOLANT_FLOOD_PIN       GPIO_NUM_23

// Automatic Tool Change
#define USER_DIGITAL_PIN_0   	GPIO_NUM_19 	// Lock
#define USER_DIGITAL_PIN_1   	GPIO_NUM_18 	// Blow
#define USER_DIGITAL_PIN_2   	GPIO_NUM_5  	// Door