/*
  Spindle_Laser.cpp

  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
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

#include "Spindle.h"
#include "Spindle_Laser.h"

namespace Spindles
{
    void Laser::Initialize()
    {
        // Initialize and configure as a normal PWM spindle first.
        PWM::Initialize();

        if (fOutputPin == UNDEFINED_PIN)
        {
            // Base initialization failed (no output pin configured).
            return;
        }

        // Lasers typically do not use spin-up/spin-down delays.
        fSpinUpDelay = 0;
        fSpinDownDelay = 0;

        // For a laser, "RPM" is used as laser power.
        // Clamp by configured full power setting.
        fMinRPM = 0;
        fMaxRPM = settings_spindle_rpm_max->get();

        // Keep a deterministic state.
        fCurrentState = SpindleState::Disable;
    }

    bool Laser::inLaserMode()
    {
        // Laser mode is controlled by a runtime setting.
        return settings_spindle_laser_mode->get();
    }

    void Laser::Dispose()
    {
        // Deinitialize using PWM spindle behavior (resets SPINDLE_* pins).
        PWM::Dispose();
    }
}