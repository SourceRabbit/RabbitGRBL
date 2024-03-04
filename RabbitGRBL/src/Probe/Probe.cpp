/*
  Probe.c

  Copyright (c) 2024 Nikolaos Siatras
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

#include "../Grbl.h"
#include "Probe.h"

// Private (Statics)
bool Probe::fIsProbeAway = false;
volatile bool Probe::fSystemIsUsingProbe = false;

/**
 * Initializes the machine's probe
 */
void Probe::Initialize()
{
    if (PROBE_PIN != UNDEFINED_PIN)
    {
#ifdef DISABLE_PROBE_PIN_PULL_UP
        pinMode(PROBE_PIN, INPUT);
#else
        pinMode(PROBE_PIN, INPUT_PULLUP); // Enable internal pull-up resistors. Normal high operation.
#endif
        //grbl_msg_sendf(MsgLevel::Info, "Probe on pin %s", pinName(PROBE_PIN).c_str());
    }
}

/**
 * Sets the probe's direction
 */
void Probe::setDirection(bool isAway)
{
    Probe::fIsProbeAway = isAway;
}

/**
 * Returns the probe pin state.
 * Triggered = true.
 * This method is called from gcode parser and probe state monitor
 */
bool Probe::isTriggered()
{
    return (PROBE_PIN == UNDEFINED_PIN) ? false : digitalRead(PROBE_PIN) ^ probe_invert->get();
}

// Monitors probe pin state and records the system position when detected. Called by the
// stepper ISR per ISR tick.
// NOTE: This function must be extremely efficient as to not bog down the stepper ISR.
void Probe::StateMonitor()
{
    if (Probe::isTriggered() ^ Probe::fIsProbeAway)
    {
        Probe::fSystemIsUsingProbe = false;
        memcpy(sys_probe_position, sys_position, sizeof(sys_position));
        sys_rt_exec_state.bit.motionCancel = true;
    }
}

/**
 * Returns true if the System is using Probe
 */
bool Probe::isSystemUsingProbe()
{
    return Probe::fSystemIsUsingProbe;
}

/**
 * Set's the probe system state value.
 * Used to coordinate the probing cycle with stepper ISR.
 */
void Probe::setSystemProbeState(bool state)
{
    Probe::fSystemIsUsingProbe = state;
}