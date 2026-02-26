/*
  Spindle_Laser.cpp

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
  Website: https://www.sourcerabbit.com

  Rabbit Rabbit GRBL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Rabbit Rabbit GRBL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Spindle.h"
#include "Spindle_Laser.h"

void Spindle_Laser::Initialize()
{
    // Initialize and configure as a normal PWM spindle first.
    Spindle_PWM::Initialize();

    if (fOutputPin == UNDEFINED_PIN)
    {
        // Base initialization failed (no output pin configured).
        return;
    }

    // Lasers typically do not use spin-up/spin-down delays.
    fSpinUpDelayMs = 0;
    fSpinDownDelayMs = 0;

    // For a laser, "RPM" is used as laser power.
    // Clamp by configured full power setting.
    fMinRPM = 0;
    fMaxRPM = settings_spindle_rpm_max->get();

    // Safe initial state
    setRPM(0);
    Stop();

    MessageSender::SendMessage(EMessageLevel::Info,
                               "Laser on Pin:%d Off:%.1f%% Min:%.1f%% Max:%.1f%% Freq:%dHz Res:%dbits",
                               fOutputPin,
                               settings_spindle_pwm_off_value->get(),
                               settings_spindle_pwm_min_value->get(),
                               settings_spindle_pwm_max_value->get(),
                               fPWMFrequency,
                               fPWMPrecision);
}

bool Spindle_Laser::inLaserMode()
{
    // Laser mode is controlled by a runtime setting.
    return settings_spindle_laser_mode->get();
}

void Spindle_Laser::Dispose()
{
    // Deinitialize using PWM spindle behavior (resets SPINDLE_* pins).
    Spindle_PWM::Dispose();
}