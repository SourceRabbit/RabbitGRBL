#pragma once
/*
        Spindle.h

        Copyright (c) 2026 Nikolaos Siatras
        Twitter: nsiatras
        Github: https://github.com/nsiatras
        Website: https://www.sourcerabbit.com

        Rabbit GRBL is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        Rabbit GRBL  is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstdint>
#include "ESpindleType.h"
#include "../Grbl.h"

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

                virtual void Initialize();
                virtual void Synch(SpindleState state, uint32_t rpm);
                virtual void Stop() = 0;
                virtual void Dispose();

                virtual bool inLaserMode();
                virtual bool isReversable();
                virtual uint32_t setRPM(uint32_t rpm) = 0;
                virtual void setState(SpindleState state, uint32_t rpm) = 0;
                virtual SpindleState getState() = 0;

                // Delays
                uint32_t getSpinUpDelay();
                uint32_t getSpinDownDelay();
                bool isUsingDelays();

                volatile SpindleState fCurrentState = SpindleState::Disable;

                static void Select();

        protected:
                uint8_t fOutputPin;
                uint8_t fEnablePin;
                uint8_t fDirectionPin;

                uint32_t fMinRPM;
                uint32_t fMaxRPM;

                // Delays
                uint32_t fSpinUpDelay;
                uint32_t fSpinDownDelay;
        };
}

extern Spindles::Spindle *fSpindle;
