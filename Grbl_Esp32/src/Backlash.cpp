/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Backlash.h"

#define DIR_POSITIVE     0
#define DIR_NEGATIVE     1
#define DIR_NEUTRAL      2

// This array holds the backlash that has been added to each axes
// in mm. The system_convert_axis_steps_to_mpos uses this array to remove the backlash added 
// in order to keep the absolute machine position (position without any backlash distance added)
float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

float backlash_compensation_to_remove_after_position_reset[MAX_N_AXIS];

static float previous_targets[MAX_N_AXIS] = {0.000};
static uint8_t target_directions[MAX_N_AXIS] = {DIR_NEGATIVE};

void backlash_ini() 
{
	// The backlash_ini method is called from Grbl.cpp
    for (int i = 0; i < MAX_N_AXIS; i++) 
	{
        previous_targets[i] = 0.000;
		backlash_compensation_to_remove_after_position_reset[i] = 0.000;
        target_directions[i] = DIR_NEUTRAL;
    }
}

float backlash_CompensateBacklashToTarget(int axis, float target) 
{

    // This method will run only if the axis has backlash setting set  >  0.
    if (axis_settings[axis]->backlash->get() > 0) 
	{
        if (target > previous_targets[axis]) 
		{
            // The Machine is moving "positive" compared to previous move
            // If the last move was "negative" add backlash compensation to the target
            if (target_directions[axis] != DIR_POSITIVE) 
			{
                target = target + axis_settings[axis]->backlash->get();
                backlash_compensation_to_remove_from_mpos[axis] += axis_settings[axis]->backlash->get();
            }
			
            target_directions[axis] = DIR_POSITIVE;
        } 
		else if (target < previous_targets[axis]) 
		{
            // The Machine is moving "negative" compared to previous move
            // If the last move was "positive" remove backlash compensation from the target
            if (target_directions[axis] != DIR_NEGATIVE) 
			{
                target = target - axis_settings[axis]->backlash->get();
                backlash_compensation_to_remove_from_mpos[axis] -= axis_settings[axis]->backlash->get();
            }

            target_directions[axis] = DIR_NEGATIVE;
        }

        // Update previous target to current target
        previous_targets[axis] = target;
    }

    return target;
}

void backlash_ResetBacklashCompensationAddedToAxis() 
{
    // RESET BACKLASH
    for (int i = 0; i < MAX_N_AXIS; i++) 
	{
		backlash_compensation_to_remove_after_position_reset[i] = backlash_compensation_to_remove_from_mpos[i];
        //backlash_compensation_to_remove_from_mpos[i] = 0;
        //target_directions[i] = DIR_NEGATIVE;
    }
}