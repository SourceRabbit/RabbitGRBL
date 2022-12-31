/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Backlash.h"

#define DIR_POSITIVE 0
#define DIR_NEGATIVE 1
#define DIR_NEUTRAL 2

bool backlash_compensation_motion_created = false;
static float previous_targets[MAX_N_AXIS] = {0.000};
static uint8_t axis_directions[MAX_N_AXIS] = {DIR_NEGATIVE};

// This array holds the amount of millimeters that has been added to each axes
// in order to remove backlash.
// The system_convert_axis_steps_to_mpos (located in System.cpp) uses this array to remove the backlash added
// in order to keep the absolute machine position (position without any backlash distance added)
float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

void backlash_ini()
{
    // The backlash_ini method is called from Grbl.cpp
    for (int i = 0; i < MAX_N_AXIS; i++)
    {
        previous_targets[i] = 0.000;
        backlash_compensation_to_remove_from_mpos[i] = 0.000;
        axis_directions[i] = DIR_NEUTRAL;
    }
}

/**
 * Creates a backlash-compensation target for the given axis and
 * the given target.
 *
 * */
float backlash_CreateBacklashCompensationTarget(int axis, float target)
{
    float result = previous_targets[axis];

    // This method will run only if the axis has backlash setting set > 0.
    if (axis_settings[axis]->backlash->get() > 0)
    {
        if (target > result)
        {
            // The new axis target is "Positive" compared to the previous one.
            // If the last axis target was "negative or neutral" then add backlash compensation to the result.
            if (axis_directions[axis] == DIR_NEGATIVE)
            {
                result += axis_settings[axis]->backlash->get();
                backlash_compensation_motion_created = true;
                backlash_compensation_to_remove_from_mpos[axis] += axis_settings[axis]->backlash->get();
            }

            axis_directions[axis] = DIR_POSITIVE;
        }
        else if (target < result)
        {
            // The new axis target is "Negative" compared to the previous one.
            // If the last axis target was "positive or neutral" then remove backlash compensation from the result.
            if (axis_directions[axis] == DIR_POSITIVE)
            {
                result -= axis_settings[axis]->backlash->get();
                backlash_compensation_motion_created = true;
                backlash_compensation_to_remove_from_mpos[axis] -= axis_settings[axis]->backlash->get();
            }

            axis_directions[axis] = DIR_NEGATIVE;
        }

        // Update previous target to current target
        previous_targets[axis] = target;
    }

    return result;
}

/**
 * The backlash_reset_targets is been called from limits_go_home
 * */
void backlash_reset_targets(float target[])
{
    for (int i = 0; i < MAX_N_AXIS; i++)
    {
        previous_targets[i] = target[i];
    }
}

void backlash_synch_position()
{
    // Update target_prev
    system_convert_array_steps_to_mpos(previous_targets, sys_position);
    for (int i = 0; i < MAX_N_AXIS; i++)
    {
        previous_targets[i] = previous_targets[i] - backlash_compensation_to_remove_from_mpos[i];
    }
}