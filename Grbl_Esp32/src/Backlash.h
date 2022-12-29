#pragma once

/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Grbl.h"


// This array contains the backlash that has beed added to an axes in order
// to remove it later from the Report (System.cpp--> system_convert_axis_steps_to_mpos)
extern float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

void backlash_ini();

float backlash_CompensateBacklashToTarget(int axis, float target);

void backlash_ResetBacklashCompensationAddedToAxis();