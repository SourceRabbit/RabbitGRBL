/*
    RelaySpindle.h

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

#include "Spindle.h"

namespace Spindles
{
    class Relay : public Spindle
    {
    public:
        Relay() = default;

        Relay(const Relay &) = delete;
        Relay(Relay &&) = delete;
        Relay &operator=(const Relay &) = delete;
        Relay &operator=(Relay &&) = delete;

        void Initialize() override;
        uint32_t setRPM(uint32_t rpm) override;
        void setState(SpindleState state, uint32_t rpm) override;
        SpindleState getState() override;
        void Stop() override;
        bool isReversable() override;

        ~Relay() override = default;

    protected:
        // Tracks whether the relay output is currently ON.
        bool fOutputOn = false;

        void setRelayOutput(bool on);
        void setEnablePinValue(bool enabled);
        void setDirectionPinValue(bool clockwise);
        void Dispose() override;
    };
}