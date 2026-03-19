/*
  Probe.cpp

  Copyright (c) 2024 Nikolaos Siatras
  Twitter: nsiatras
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
#include "Probe.h"

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
        // MessageSender::SendMessage(EMessageLevel::Info, "Probe on pin %s", pinName(PROBE_PIN).c_str());
    }

    Controller::getProbe().setSystemProbeState(false);
}

/**
 * Sets the probe's direction
 */
void Probe::setDirection(bool isAway)
{
    fIsProbeAway = isAway;
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
    if (isTriggered() ^ fIsProbeAway)
    {
        fSystemIsUsingProbe = false;
        CoordinatesManager::UpdateCoordinateFromSystemPosition(ECoordinate::PRB);
        sys_rt_exec_state.bit.motionCancel = true;
    }
}

/**
 * Prints current probe parameters. Upon a probe command, these parameters are updated upon a
 * successful probe or upon a failed probe with the G38.3 without errors command (if supported).
 * These values are retained until Grbl is power-cycled, whereby they will be re-zeroed.
 */
void Probe::ReportProbeParameters()
{
    float print_position[MAX_N_AXIS];
    char probe_rpt[(MAX_AXES_STRING_LENGTH + 13 + 6 + 1)]; // the probe report we are building here
    char temp[MAX_AXES_STRING_LENGTH];

    // Initialize the string with the first characters
    strcpy(probe_rpt, "[PRB:");

    // Copy probe position from CoordinatesManager into a local float array
    // (required because report_util_axis_values expects float*, not const float*)
    CoordinatesManager::getCoordinates(ECoordinate::PRB)->get(print_position);
    report_util_axis_values(print_position, temp);
    strcat(probe_rpt, temp);

    // Add the success indicator and closing characters
    sprintf(temp, ":%d]\r\n", fProbeSucceeded);
    strcat(probe_rpt, temp);

    // Send the report
    ConnectionManager::Write(probe_rpt);
}

/**
 * Returns true if the System is using Probe
 */
bool Probe::isSystemUsingProbe()
{
    return fSystemIsUsingProbe;
}

/**
 * Sets the probe system state value.
 * Used to coordinate the probing cycle with stepper ISR.
 */
void Probe::setSystemProbeState(bool state)
{
    fSystemIsUsingProbe = state;
}

/**
 * Returns true if the last probing cycle was successful.
 */
bool Probe::getProbeSucceeded()
{
    return fProbeSucceeded;
}

/**
 * Sets the probe succeeded flag.
 */
void Probe::setProbeSucceeded(bool succeeded)
{
    fProbeSucceeded = succeeded;
}