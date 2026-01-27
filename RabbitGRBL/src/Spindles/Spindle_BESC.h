#pragma once

/*
    Spindle_BESC.h

    Copyright (c) 2026 Nikolaos Siatras
    Twitter: nsiatras
    Github: https://github.com/nsiatras
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
    along with Rabbit GRBL. If not, see <http://www.gnu.org/licenses/>.
*/

/*
    This a special type of PWM spindle for RC type Brushless DC Speed
    controllers. They use a short pulse for off and a longer pulse for
    full on. The pulse is always a small portion of the full cycle.
    Some BESCs have a special turn on procedure. This may be a one time
    procedure or must be done every time. The user must do that via gcode.

    Important ESC Settings:
    - 50 Hz is a typical frequency for an ESC
    - Some ESCs can handle higher frequencies, but there is no advantage
    - BESC_MIN_PULSE_SECS is typically 1ms (0.001 sec) or less
    - BESC_MAX_PULSE_SECS is typically 2ms (0.002 sec) or more
*/

#include "Spindle_PWM.h"

// Don't change these
#define BESC_PWM_FREQ 50.0 // Hz
#define BESC_PULSE_PERIOD (1.0 / BESC_PWM_FREQ)

// Ok to tweak. These are the pulse lengths in seconds
// #define them in your machine definition file if you want different values
#ifndef BESC_MIN_PULSE_SECS
#define BESC_MIN_PULSE_SECS 0.0009f // in seconds
#endif

#ifndef BESC_MAX_PULSE_SECS
#define BESC_MAX_PULSE_SECS 0.0022f // in seconds
#endif

// Calculations...don't change
#define BESC_MIN_PULSE_CNT static_cast<uint16_t>(BESC_MIN_PULSE_SECS / BESC_PULSE_PERIOD * 65535.0)
#define BESC_MAX_PULSE_CNT static_cast<uint16_t>(BESC_MAX_PULSE_SECS / BESC_PULSE_PERIOD * 65535.0)

namespace Spindles
{
    class BESC : public PWM
    {
    public:
        BESC() = default;

        BESC(const BESC &) = delete;
        BESC(BESC &&) = delete;
        BESC &operator=(const BESC &) = delete;
        BESC &operator=(BESC &&) = delete;

        void Initialize() override;

        ~BESC() override = default;
    };
}