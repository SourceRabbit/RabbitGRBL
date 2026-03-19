/*
  ECoordinate.h

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
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
  along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

// Enum representing the non-persistent coordinate slots managed by CoordinatesManager.
// These are not saved to NVS and are reset on every power cycle.
enum class ECoordinate : uint8_t
{
    WPos = 0, // Current work position (tracked by the G-code interpreter)
    PRB,      // Last triggered probe position (updated on probe trigger)

    Ended,      // Sentinel value - equals the total number of ECoordinate slots
};

// Allow iteration over ECoordinate values
inline ECoordinate &operator++(ECoordinate &i)
{
    i = static_cast<ECoordinate>(static_cast<uint8_t>(i) + 1);
    return i;
}