/*
  Spindle_BESC.cpp

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
#include "Spindle_BESC.h"

namespace Spindles
{
    void BESC::Initialize()
    {
        // Call base PWM Spindle initialization to resolve pins and basic settings
        PWM::Initialize();

        if (fOutputPin == UNDEFINED_PIN)
        {
            MessageSender::SendMessage(EMessageLevel::Info, "Warning: BESC output pin not defined");
            return; // Cannot continue without output pin
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize PWM Output for BESC ///////////////////////////////////////////////////////////////////////////////////////////////////
        const uint8_t channelNumber = 0; // Channel 0 is reserved for spindle use
        const uint32_t pwmFrequency = static_cast<uint32_t>(BESC_PWM_FREQ);
        const uint8_t pwmPrecision = 16;                  // BESC requires 16-bit precision
        const uint32_t pwmPeriod = (1UL << pwmPrecision); // 65536
        const uint32_t pwmOffValue = BESC_MIN_PULSE_CNT;
        const uint32_t pwmMinValue = BESC_MIN_PULSE_CNT;
        const uint32_t pwmMaxValue = BESC_MAX_PULSE_CNT;
        const bool invertPWM = false; // BESC typically doesn't need inversion
        InitializePWMOutput(channelNumber, pwmFrequency, pwmPrecision, pwmPeriod, pwmOffValue, pwmMinValue, pwmMaxValue, invertPWM);
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        // Safe initial state
        setRPM(0);
        Stop();

        MessageSender::SendMessage(EMessageLevel::Info,
                       "BESC spindle on Pin:%d Min:%.2fms Max:%.2fms Freq:%dHz Res:%dbits",
                       fOutputPin,
                       BESC_MIN_PULSE_SECS * 1000.0, // Convert to milliseconds
                       BESC_MAX_PULSE_SECS * 1000.0, // Convert to milliseconds
                       fPWMFrequency,
                       fPWMPrecision);
    }

}