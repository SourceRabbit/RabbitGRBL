#pragma once

/*
  PWMSpindle.h

  Copyright (c) 2026 Nikolaos Siatras
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

namespace Spindles
{
    // This adds support for PWM
    class PWM : public Spindle
    {
    public:
        PWM() = default;

        PWM(const PWM &) = delete;
        PWM(PWM &&) = delete;
        PWM &operator=(const PWM &) = delete;
        PWM &operator=(PWM &&) = delete;

        void Initialize() override;
        virtual uint32_t setRPM(uint32_t rpm) override;
        void setState(SpindleState state, uint32_t rpm) override;
        SpindleState getState() override;
        void Stop() override;

        virtual ~PWM() {}

    protected:
        int32_t fCurrentPWMDuty;
        uint32_t fMinRPM;
        uint32_t fMaxRPM;

        uint8_t fPWMChannelNumber;
        bool fInvertPWM;
        uint32_t fPWMOffValue;
        uint32_t fPWMMinValue;
        uint32_t fPWMMaxValue;
        uint32_t fPWMFrequency;
        uint32_t fPWMPeriod; // how many counts in 1 period
        uint8_t fPWMPrecision;

        virtual void setPWMOutput(uint32_t duty);
        virtual void setEnablePinValue(bool active);
        virtual void setDirectionPinValue(bool Clockwise);
        virtual void Dispose();

        uint8_t CalculatePWMPrecision(uint32_t freq);
    };
}
