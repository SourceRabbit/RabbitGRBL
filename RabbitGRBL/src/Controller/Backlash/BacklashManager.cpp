/*
  BacklashManager.cpp

  Copyright (c) 2024 Nikolaos Siatras
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

#include "BacklashManager.h"

#define DIR_POSITIVE 0
#define DIR_NEGATIVE 1
#define DIR_NEUTRAL 2

/**
 * Initialize the Backlash Manager
 * This method is called from Controller::Initialize (Controller.cpp)
 */
void BacklashManager::Initialize()
{
    for (int i = 0; i < MAX_N_AXIS; i++)
    {
        fPreviousTargets[i] = 0.0;
        fAxisDirections[i] = DIR_NEUTRAL;
    }

    ResetTargets();
}

/**
 * Plans and queues a backlash motion into planner buffer
 */
void BacklashManager::CompensateBacklash(float *target, plan_line_data_t *pl_data)
{
    float backlash_compensation_target[MAX_N_AXIS] = {0.0};
    bool perform_backlash_compensation_motion = false;

    for (int axis = 0; axis < MAX_N_AXIS; axis++)
    {
        backlash_compensation_target[axis] = fPreviousTargets[axis];

        if (axis_settings[axis]->backlash->get() > 0)
        {
            if (target[axis] > fPreviousTargets[axis])
            {
                // The new axis target is "Positive" compared to the previous one.
                // If the last axis target was "Negative" then alter the backlash_compensation_target for this axis.
                if (fAxisDirections[axis] != DIR_POSITIVE)
                {
                    backlash_compensation_target[axis] += axis_settings[axis]->backlash->get();
                    perform_backlash_compensation_motion = true;
                }

                fAxisDirections[axis] = DIR_POSITIVE;
            }
            else if (target[axis] < fPreviousTargets[axis])
            {
                // The new axis target is "Negative" compared to the previous one.
                // If the last axis target was "Positive" then alter the backlash_compensation_target for this axis.
                if (fAxisDirections[axis] != DIR_NEGATIVE)
                {
                    backlash_compensation_target[axis] -= axis_settings[axis]->backlash->get();
                    perform_backlash_compensation_motion = true;
                }

                fAxisDirections[axis] = DIR_NEGATIVE;
            }
        }
    }

    if (perform_backlash_compensation_motion)
    {
        // MessageSender::SendMessage(EMessageLevel::Info, "Anti backlash Motion");

        // Queue the backlash motion into planner buffer
        plan_line_data_t pl_backlash_data;
        plan_line_data_t *backlash_data = &pl_backlash_data;
        memset(backlash_data, 0, sizeof(plan_line_data_t)); // Zero backlash_data struct

        backlash_data->spindle = pl_data->spindle;
        backlash_data->spindle_speed = pl_data->spindle_speed;
        backlash_data->feed_rate = pl_data->feed_rate < MINIMUM_FEED_RATE ? 20000 : pl_data->feed_rate;
        backlash_data->coolant = pl_data->coolant;
        backlash_data->motion = {};
        backlash_data->motion.antiBacklashMotion = 1;

        do
        {
            protocol_execute_realtime(); // Check for any run-time commands
            if (sys.abort)
            {
                return; // Bail, if system abort.
            }

            if (plan_check_full_buffer())
            {
                protocol_auto_cycle_start(); // Auto-cycle start when buffer is full.
            }
            else
            {
                break;
            }
        } while (1);

        // Plan and queue the backlash motion into planner buffer
        plan_buffer_line(backlash_compensation_target, &pl_backlash_data);
    }

    // Update fPreviousTargets[] values to target[] values
    memcpy(fPreviousTargets, target, sizeof(float) * MAX_N_AXIS); // target_prev[] = target[]
}

/**
 * Resets the backlash targets. Called from mc_homing_cycle.
 */
void BacklashManager::ResetTargets()
{
    // Make axis directions Neutral
    for (int i = 0; i < MAX_N_AXIS; i++)
    {
        fPreviousTargets[i] = 0.0;
        fAxisDirections[i] = DIR_NEUTRAL;
    }

    system_convert_array_steps_to_mpos(fPreviousTargets, sys_position);
}

/**
 * Synchronizes backlash position while using probe. Called from mc_probe_cycle.
 */
void BacklashManager::SynchPositionWhileUsingProbe()
{
    // Update target_prev
    system_convert_array_steps_to_mpos(fPreviousTargets, sys_position);
}
