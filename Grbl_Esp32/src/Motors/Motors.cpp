/*
        Motors.cpp
        Part of Grbl_ESP32
        2020 -	Bart Dring
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

#include "Motors.h"

#include "Motor.h"
#include "../Grbl.h"

#include "NullMotor.h"
#include "StandardStepper.h"
#include "RcServo.h"

Motors::Motor *myMotor[MAX_AXES][MAX_GANGED]; // number of axes (normal and ganged)
void init_motors()
{
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Init Motors");

        auto n_axis = number_axis->get();

        if (n_axis >= 1)
        {

#ifdef defined(X_SERVO_PIN)
                myMotor[X_AXIS][0] = new Motors::RcServo(X_AXIS, X_SERVO_PIN);
#elif defined(X_STEP_PIN)
                myMotor[X_AXIS][0] = new Motors::StandardStepper(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_DISABLE_PIN);
#else
                myMotor[X_AXIS][0] = new Motors::Nullmotor(X_AXIS);
#endif

#if defined(X2_STEP_PIN)
                myMotor[X_AXIS][1] = new Motors::StandardStepper(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_DISABLE_PIN);
#else
                myMotor[X_AXIS][1] = new Motors::Nullmotor(X2_AXIS);
#endif
        }
        else
        {
                myMotor[X_AXIS][0] = new Motors::Nullmotor(X_AXIS);
                myMotor[X_AXIS][1] = new Motors::Nullmotor(X2_AXIS);
        }

        if (n_axis >= 2)
        {
                // this WILL be done better with settings

#ifdef defined(Y_SERVO_PIN)
                myMotor[Y_AXIS][0] = new Motors::RcServo(Y_AXIS, Y_SERVO_PIN);
#elif defined(Y_STEP_PIN)
                myMotor[Y_AXIS][0] = new Motors::StandardStepper(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_DISABLE_PIN);
#else
                myMotor[Y_AXIS][0] = new Motors::Nullmotor(Y_AXIS);
#endif

#if defined(Y2_STEP_PIN)
                myMotor[Y_AXIS][1] = new Motors::StandardStepper(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_DISABLE_PIN);
#else
                myMotor[Y_AXIS][1] = new Motors::Nullmotor(Y2_AXIS);
#endif
        }
        else
        {
                myMotor[Y_AXIS][0] = new Motors::Nullmotor(Y_AXIS);
                myMotor[Y_AXIS][1] = new Motors::Nullmotor(Y2_AXIS);
        }

        if (n_axis >= 3)
        {
                // this WILL be done better with settings

#ifdef defined(Z_SERVO_PIN)
                myMotor[Z_AXIS][0] = new Motors::RcServo(Z_AXIS, Z_SERVO_PIN);
#elif defined(Z_STEP_PIN)
                myMotor[Z_AXIS][0] = new Motors::StandardStepper(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_DISABLE_PIN);
#else
                myMotor[Z_AXIS][0] = new Motors::Nullmotor(Z_AXIS);
#endif

#if defined(Z2_STEP_PIN)
                myMotor[Z_AXIS][1] = new Motors::StandardStepper(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_DISABLE_PIN);
#else
                myMotor[Z_AXIS][1] = new Motors::Nullmotor(Z2_AXIS);
#endif
        }
        else
        {
                myMotor[Z_AXIS][0] = new Motors::Nullmotor(Z_AXIS);
                myMotor[Z_AXIS][1] = new Motors::Nullmotor(Z2_AXIS);
        }

        if (n_axis >= 4)
        {
                // this WILL be done better with settings

#ifdef defined(A_SERVO_PIN)
                myMotor[A_AXIS][0] = new Motors::RcServo(A_AXIS, A_SERVO_PIN);
#elif defined(A_STEP_PIN)
                myMotor[A_AXIS][0] = new Motors::StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_DISABLE_PIN);
#else
                myMotor[A_AXIS][0] = new Motors::Nullmotor(A_AXIS);
#endif

#if defined(A2_STEP_PIN)
                myMotor[A_AXIS][1] = new Motors::StandardStepper(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_DISABLE_PIN);
#else
                myMotor[A_AXIS][1] = new Motors::Nullmotor(A2_AXIS);
#endif
        }
        else
        {
                myMotor[A_AXIS][0] = new Motors::Nullmotor(A_AXIS);
                myMotor[A_AXIS][1] = new Motors::Nullmotor(A2_AXIS);
        }

        if (n_axis >= 5)
        {
                // this WILL be done better with settings
#ifdef defined(B_SERVO_PIN)
                myMotor[B_AXIS][0] = new Motors::RcServo(B_AXIS, B_SERVO_PIN);
#elif defined(B_STEP_PIN)
                myMotor[B_AXIS][0] = new Motors::StandardStepper(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_DISABLE_PIN);
#else
                myMotor[B_AXIS][0] = new Motors::Nullmotor(B_AXIS);
#endif

#if defined(B2_STEP_PIN)
                myMotor[B_AXIS][1] = new Motors::StandardStepper(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_DISABLE_PIN);
#else
                myMotor[B_AXIS][1] = new Motors::Nullmotor(B2_AXIS);
#endif
        }
        else
        {
                myMotor[B_AXIS][0] = new Motors::Nullmotor(B_AXIS);
                myMotor[B_AXIS][1] = new Motors::Nullmotor(B2_AXIS);
        }

        if (n_axis >= 6)
        {
                // this WILL be done better with settings
#if defined(C_SERVO_PIN)
                myMotor[C_AXIS][0] = new Motors::RcServo(C_AXIS, C_SERVO_PIN);
#elif defined(C_STEP_PIN)
                myMotor[C_AXIS][0] = new Motors::StandardStepper(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_DISABLE_PIN);
#else
                myMotor[C_AXIS][0] = new Motors::Nullmotor(C_AXIS);
#endif

#if defined(C2_STEP_PIN)
                myMotor[C_AXIS][1] = new Motors::StandardStepper(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_DISABLE_PIN);
#else
                myMotor[C_AXIS][1] = new Motors::Nullmotor(C2_AXIS);
#endif
        }
        else
        {
                myMotor[C_AXIS][0] = new Motors::Nullmotor(C_AXIS);
                myMotor[C_AXIS][1] = new Motors::Nullmotor(C2_AXIS);
        }

        if (STEPPERS_DISABLE_PIN != UNDEFINED_PIN)
        {
                pinMode(STEPPERS_DISABLE_PIN, OUTPUT); // global motor enable pin
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Global stepper disable pin:%s", pinName(STEPPERS_DISABLE_PIN));
        }

        // certain motors need features to be turned on. Check them here
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
        {
                for (uint8_t gang_index = 0; gang_index < 2; gang_index++)
                {
                        myMotor[axis][gang_index]->init();
                }
        }
}

void motors_set_disable(bool disable, uint8_t mask)
{
        static bool prev_disable = true;
        static uint8_t prev_mask = 0;

        if ((disable == prev_disable) && (mask == prev_mask))
        {
                return;
        }

        prev_disable = disable;
        prev_mask = mask;

        if (step_enable_invert->get())
        {
                disable = !disable; // Apply pin invert.
        }

        // now loop through all the motors to see if they can individually disable
        auto n_axis = number_axis->get();
        for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++)
        {
                for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
                {
                        if (bitnum_istrue(mask, axis))
                        {
                                myMotor[axis][gang_index]->set_disable(disable);
                        }
                }
        }

        // global disable.
        digitalWrite(STEPPERS_DISABLE_PIN, disable);

        // Add an optional delay for stepper drivers. that need time
        // Some need time after the enable before they can step.
        auto wait_disable_change = enable_delay_microseconds->get();
        if (wait_disable_change != 0)
        {
                auto disable_start_time = esp_timer_get_time() + wait_disable_change;

                while ((esp_timer_get_time() - disable_start_time) < 0)
                {
                        NOP();
                }
        }
}

void motors_read_settings()
{
        // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Read Settings");
        auto n_axis = number_axis->get();
        for (uint8_t gang_index = 0; gang_index < 2; gang_index++)
        {
                for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
                {
                        myMotor[axis][gang_index]->read_settings();
                }
        }
}

// use this to tell all the motors what the current homing mode is
// They can use this to setup things like Stall
uint8_t motors_set_homing_mode(uint8_t homing_mask, bool isHoming)
{
        uint8_t can_home = 0;
        auto n_axis = number_axis->get();
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
        {
                if (bitnum_istrue(homing_mask, axis))
                {
                        if (myMotor[axis][0]->set_homing_mode(isHoming))
                        {
                                bitnum_true(can_home, axis);
                        }
                        myMotor[axis][1]->set_homing_mode(isHoming);
                }
        }
        return can_home;
}

bool motors_direction(uint8_t dir_mask)
{
        auto n_axis = number_axis->get();
        // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "motors_set_direction_pins:0x%02X", onMask);

        // Set the direction pins, but optimize for the common
        // situation where the direction bits haven't changed.
        static uint8_t previous_dir = 255; // should never be this value
        if (dir_mask != previous_dir)
        {
                previous_dir = dir_mask;

                for (int axis = X_AXIS; axis < n_axis; axis++)
                {
                        bool thisDir = bitnum_istrue(dir_mask, axis);
                        myMotor[axis][0]->set_direction(thisDir);
                        myMotor[axis][1]->set_direction(thisDir);
                }

                return true;
        }
        else
        {
                return false;
        }
}

void motors_step(uint8_t step_mask)
{
        auto n_axis = number_axis->get();
        // grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "motors_set_direction_pins:0x%02X", onMask);

        // Turn on step pulses for motors that are supposed to step now
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
        {
                if (bitnum_istrue(step_mask, axis))
                {
                        if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::A))
                        {
                                myMotor[axis][0]->step();
                        }
                        if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::B))
                        {
                                myMotor[axis][1]->step();
                        }
                }
        }
}
// Turn all stepper pins off
void motors_unstep()
{
        auto n_axis = number_axis->get();
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
        {
                myMotor[axis][0]->unstep();
                myMotor[axis][1]->unstep();
        }
}
