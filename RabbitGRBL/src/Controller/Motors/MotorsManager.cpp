/*
    MotorsManager.cpp

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

#include "../../Grbl.h"
#include "MotorsManager.h"

// RMT or Software...
#ifdef USE_SOFTWARE_STEPS
using StepperImpl = Stepper_Software;
#else
using StepperImpl = Stepper_RMT;
#endif

void MotorsManager::Initialize()
{
    if (fInitialized)
    {
        return;
    }

    // MessageSender::SendMessage(EMessageLevel::Info, "Init Motors");

    auto n_axis = number_axis->get();

    /////////////////////////////////////////////////////////////////////////////////
    // X AXIS
    /////////////////////////////////////////////////////////////////////////////////
    if (n_axis >= 1)
    {
#if defined(X_STEP_PIN)
        fMotors[X_AXIS][0] = new StepperImpl(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_DISABLE_PIN);
#else
        fMotors[X_AXIS][0] = new Nullmotor(X_AXIS);
#endif

#if defined(X2_STEP_PIN)
        fMotors[X_AXIS][1] = new StepperImpl(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_DISABLE_PIN);
#else
        fMotors[X_AXIS][1] = new Nullmotor(X2_AXIS);
#endif
    }
    else
    {
        fMotors[X_AXIS][0] = new Nullmotor(X_AXIS);
        fMotors[X_AXIS][1] = new Nullmotor(X2_AXIS);
    }

    /////////////////////////////////////////////////////////////////////////////////
    // Y AXIS
    /////////////////////////////////////////////////////////////////////////////////
    if (n_axis >= 2)
    {
#if defined(Y_STEP_PIN)
        fMotors[Y_AXIS][0] = new StepperImpl(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_DISABLE_PIN);
#else
        fMotors[Y_AXIS][0] = new Nullmotor(Y_AXIS);
#endif

#if defined(Y2_STEP_PIN)
        fMotors[Y_AXIS][1] = new StepperImpl(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_DISABLE_PIN);
#else
        fMotors[Y_AXIS][1] = new Nullmotor(Y2_AXIS);
#endif
    }
    else
    {
        fMotors[Y_AXIS][0] = new Nullmotor(Y_AXIS);
        fMotors[Y_AXIS][1] = new Nullmotor(Y2_AXIS);
    }

    /////////////////////////////////////////////////////////////////////////////////
    // Z AXIS
    /////////////////////////////////////////////////////////////////////////////////
    if (n_axis >= 3)
    {
#if defined(Z_STEP_PIN)
        fMotors[Z_AXIS][0] = new StepperImpl(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_DISABLE_PIN);
#else
        fMotors[Z_AXIS][0] = new Nullmotor(Z_AXIS);
#endif

#if defined(Z2_STEP_PIN)
        fMotors[Z_AXIS][1] = new StepperImpl(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_DISABLE_PIN);
#else
        fMotors[Z_AXIS][1] = new Nullmotor(Z2_AXIS);
#endif
    }
    else
    {
        fMotors[Z_AXIS][0] = new Nullmotor(Z_AXIS);
        fMotors[Z_AXIS][1] = new Nullmotor(Z2_AXIS);
    }

    /////////////////////////////////////////////////////////////////////////////////
    // A AXIS
    /////////////////////////////////////////////////////////////////////////////////
    if (n_axis >= 4)
    {
#if defined(A_STEP_PIN)
        fMotors[A_AXIS][0] = new StepperImpl(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_DISABLE_PIN);
#else
        fMotors[A_AXIS][0] = new Nullmotor(A_AXIS);
#endif

#if defined(A2_STEP_PIN)
        fMotors[A_AXIS][1] = new StepperImpl(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_DISABLE_PIN);
#else
        fMotors[A_AXIS][1] = new Nullmotor(A2_AXIS);
#endif
    }
    else
    {
        fMotors[A_AXIS][0] = new Nullmotor(A_AXIS);
        fMotors[A_AXIS][1] = new Nullmotor(A2_AXIS);
    }

    /////////////////////////////////////////////////////////////////////////////////
    // B AXIS
    /////////////////////////////////////////////////////////////////////////////////
    if (n_axis >= 5)
    {
#if defined(B_STEP_PIN)
        fMotors[B_AXIS][0] = new StepperImpl(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_DISABLE_PIN);
#else
        fMotors[B_AXIS][0] = new Nullmotor(B_AXIS);
#endif

#if defined(B2_STEP_PIN)
        fMotors[B_AXIS][1] = new StepperImpl(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_DISABLE_PIN);
#else
        fMotors[B_AXIS][1] = new Nullmotor(B2_AXIS);
#endif
    }
    else
    {
        fMotors[B_AXIS][0] = new Nullmotor(B_AXIS);
        fMotors[B_AXIS][1] = new Nullmotor(B2_AXIS);
    }

    /////////////////////////////////////////////////////////////////////////////////
    // C AXIS
    /////////////////////////////////////////////////////////////////////////////////
    if (n_axis >= 6)
    {
#if defined(C_STEP_PIN)
        fMotors[C_AXIS][0] = new StepperImpl(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_DISABLE_PIN);
#else
        fMotors[C_AXIS][0] = new Nullmotor(C_AXIS);
#endif

#if defined(C2_STEP_PIN)
        fMotors[C_AXIS][1] = new StepperImpl(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_DISABLE_PIN);
#else
        fMotors[C_AXIS][1] = new Nullmotor(C2_AXIS);
#endif
    }
    else
    {
        fMotors[C_AXIS][0] = new Nullmotor(C_AXIS);
        fMotors[C_AXIS][1] = new Nullmotor(C2_AXIS);
    }

    /////////////////////////////////////////////////////////////////////////////////
    // STEPPERS DISABLE PIN
    /////////////////////////////////////////////////////////////////////////////////
    if (STEPPERS_DISABLE_PIN != UNDEFINED_PIN)
    {
        pinMode(STEPPERS_DISABLE_PIN, OUTPUT); // global motor enable pin
                                               // MessageSender::SendMessage(EMessageLevel::Info, "Global stepper disable pin:%s", pinName(STEPPERS_DISABLE_PIN));
    }

    // Initialize motors
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
    {
        for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++)
        {
            fMotors[axis][gang_index]->Initialize();
        }
    }

    fInitialized = true;
}

void MotorsManager::setDisable(bool disable, uint8_t mask)
{
    if ((disable == fLastMotorsState) && (mask == fLastStateMask))
    {
        return;
    }

    fLastMotorsState = disable;
    fLastStateMask = mask;

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
                fMotors[axis][gang_index]->setDisable(disable);
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

        while (esp_timer_get_time() < disable_start_time)
        {
            NOP();
        }
    }
}

// use this to tell all the motors what the current homing mode is
// They can use this to setup things like Stall
uint8_t MotorsManager::setHomingMode(uint8_t homing_mask, bool isHoming)
{
    uint8_t can_home = 0;
    auto n_axis = number_axis->get();
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
    {
        if (bitnum_istrue(homing_mask, axis))
        {
            if (fMotors[axis][0]->setHomingMode(isHoming))
            {
                bitnum_true(can_home, axis);
            }
            fMotors[axis][1]->setHomingMode(isHoming);
        }
    }
    return can_home;
}

bool MotorsManager::setMotorsDirection(uint8_t dir_mask)
{
    auto n_axis = number_axis->get();

    static uint8_t previous_dir = 255; // should never be this value
    if (dir_mask != previous_dir)
    {
        previous_dir = dir_mask;

        for (int axis = X_AXIS; axis < n_axis; axis++)
        {
            bool thisDir = bitnum_istrue(dir_mask, axis);
            fMotors[axis][0]->setDirection(thisDir);
            fMotors[axis][1]->setDirection(thisDir);
        }

#ifdef USE_SOFTWARE_STEPS
        // Software steppers need manual direction delay handling
        auto wait_direction = direction_delay_microseconds->get();

        if (wait_direction > 0)
        {
            auto direction_pulse_start_time = esp_timer_get_time() + wait_direction;
            while ((esp_timer_get_time() - direction_pulse_start_time) < 0)
            {
                NOP(); // spin here until time to turn off step
            }
        }
#endif

        return true;
    }
    else
    {
        return false;
    }
}

void MotorsManager::Step(uint8_t step_mask)
{
    auto n_axis = number_axis->get();

#ifdef USE_SOFTWARE_STEPS
    // Software steppers: record step start time for manual pulse timing
    fStepPulseTimeStart = esp_timer_get_time();
#endif

    for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
    {
        if (bitnum_istrue(step_mask, axis))
        {
            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::A))
            {
                fMotors[axis][0]->Step();
            }
            if ((ganged_mode == SquaringMode::Dual) || (ganged_mode == SquaringMode::B))
            {
                fMotors[axis][1]->Step();
            }
        }
    }
}

// Turn all stepper pins off
void MotorsManager::Unstep()
{
#ifdef USE_SOFTWARE_STEPS
    // Software steppers: busy-wait until step pulse duration has elapsed
    while (esp_timer_get_time() - fStepPulseTimeStart < pulse_microseconds->get())
    {
        NOP(); // spin here until time to turn off step
    }
#endif

    auto n_axis = number_axis->get();
    for (uint8_t axis = X_AXIS; axis < n_axis; axis++)
    {
        fMotors[axis][0]->Unstep();
        fMotors[axis][1]->Unstep();
    }
}