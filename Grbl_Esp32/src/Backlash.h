#pragma once

/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Grbl.h"

extern float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

void backlash_ini();
float backlash_CompensateBacklashToTarget(int axis, float target);
