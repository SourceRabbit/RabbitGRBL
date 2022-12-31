#pragma once

/**
 * Created by Nikos Siatras
 * Twitter: nsiatras
 * Website: https://www.sourcerabbit.com
 */

#include "Grbl.h"

extern bool backlash_compensation_motion_created;
extern float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

void backlash_ini();
float backlash_CreateBacklashCompensationTarget(int axis, float target);
