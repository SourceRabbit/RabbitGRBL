#pragma once
/*
  Backlash.cpp

  Copyright (c) 2023 Nikolaos Siatras
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

#include "Grbl.h"

extern float backlash_compensation_to_remove_from_mpos[MAX_N_AXIS];

void backlash_ini(void);
void backlash_compensate_backlash(float *target, plan_line_data_t *pl_data);

void backlash_reset_targets(void);
void backlash_synch_position_while_using_probe(void);
