/*
  Probe.h

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

#pragma once

class Probe
{
public:
    static void Initialize();
    static void setDirection(bool isAway);
    static bool isTriggered();
    static void StateMonitor();

    /*static bool getSystemProbeState();
    static void setSystemProbeState(bool state);*/

    static bool isSystemUsingProbe();
    static void setSystemProbeState(bool state);

private:
    static bool fIsProbeAway;
    static volatile bool fSystemIsUsingProbe;
};
