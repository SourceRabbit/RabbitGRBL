/*
  Probe.h

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

#pragma once

class Probe
{
public:
    void Initialize();
    void setDirection(bool isAway);
    bool isTriggered();
    void StateMonitor();

    void ReportProbeParameters();

    bool isSystemUsingProbe();
    void setSystemProbeState(bool state);

    bool getProbeSucceeded();
    void setProbeSucceeded(bool succeeded);

private:
    bool fIsProbeAway = false;
    volatile bool fSystemIsUsingProbe = false;
    bool fProbeSucceeded = false; // Tracks if last probing cycle was successful
};
