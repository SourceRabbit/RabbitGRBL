#pragma once

/*
  Spindle.h

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

#include <cstdint>
#include "../Grbl.h"

enum class ESpindleType : int8_t
{
    NONE = 0,
    PWM,
    RELAY,
    LASER,
    BESC,
};

namespace Spindles
{

    class Spindle
    {
    public:
        Spindle() = default;

        Spindle(const Spindle &) = delete;
        Spindle(Spindle &&) = delete;
        Spindle &operator=(const Spindle &) = delete;
        Spindle &operator=(Spindle &&) = delete;

        virtual ~Spindle() {}

        virtual void Initialize() = 0;
        virtual uint32_t setRPM(uint32_t rpm) = 0;
        virtual void setState(SpindleState state, uint32_t rpm) = 0;
        virtual SpindleState getState() = 0;
        virtual void Stop() = 0;
        virtual bool inLaserMode();
        virtual bool isReversable();
        virtual void Synch(SpindleState state, uint32_t rpm);
        virtual void Dispose();

       
        volatile SpindleState fCurrentState = SpindleState::Disable;

        bool fUseDelays;
        uint32_t fSpinUpDelay;
        uint32_t fSpinDownDelay;

        static void Select();
    };

}

extern Spindles::Spindle *fSpindle;
