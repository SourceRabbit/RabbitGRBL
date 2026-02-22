#pragma once

/*
  Spindle_Laser.h

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

#include "Spindle_PWM.h"

namespace Spindles
{
    // Laser spindle implementation based on the PWM spindle.
    // Uses the standard SPINDLE_* pins (no LASER_* pins).
    class Laser : public PWM
    {
    public:
        Laser() = default;

        Laser(const Laser &) = delete;
        Laser(Laser &&) = delete;
        Laser &operator=(const Laser &) = delete;
        Laser &operator=(Laser &&) = delete;

        void Initialize() override;
        void Dispose() override;
        bool inLaserMode() override;

        ~Laser() override = default;
    };
}