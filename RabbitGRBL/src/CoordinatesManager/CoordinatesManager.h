/*
  CoordinatesManager.h

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

/**
 * CoordinatesManager is a static manager class responsible for managing
 * all Work Coordinate Systems (WCS) and reference positions used by the
 * CNC controller.
 *
 * It owns and controls access to the fCoordinates array, which holds Coordinates
 * objects for G54-G59 (Work Coordinate Systems) and G28/G30 (Home Positions).
 *
 * All coordinate data is persisted in NVS (Non-Volatile Storage), ensuring
 * that coordinate values survive power cycles and reboots.
 *
 * Usage:
 *   CoordinatesManager::Initialize();                        // Call once at startup
 *   CoordinatesManager::getCoordinates()[CoordIndex::G54];   // Access a WCS
 */

#pragma once

#include "Coordinates.h"
#include "ECoordinatesIndex.h"

class CoordinatesManager
{
public:
    static void Initialize();

    static void Reset();

    static Coordinates *getCoordinates(ECoordinatesIndex index);

private:
    static Coordinates *fCoordinates[ECoordinatesIndex::End];

    static void InitializeCoordinate(ECoordinatesIndex index, const char *name);
};
