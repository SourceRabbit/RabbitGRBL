/*
  Backlash.h 

  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Website: https://www.sourcerabbit.com
*/

#pragma once

#include "Grbl.h"

void backlash_ini(void);
void backlash_compensate_backlash(float *target, plan_line_data_t *pl_data);

void backlash_reset_targets(void);
void backlash_synch_position_while_using_probe(void);
