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
 * It owns and controls access to the fOffsets array, which holds Coordinates
 * objects for G54-G59 (Work Coordinate Systems) and G28/G30 (Home Positions).
 *
 * Persistent coordinates (G54-G59, G28, G30) are stored as PersistentCoordinates
 * instances, which save and load their values to/from NVS automatically.
 *
 * Non-persistent coordinates (e.g. G92) are stored as plain Coordinates instances
 * and are not saved to NVS.
 *
 * fWorkPositionCoordinates holds the current work position as tracked by the
 * G-code interpreter. It is non-persistent and never saved to NVS.
 *
 * Usage:
 *   CoordinatesManager::Initialize();                   // Call once at startup
 *   CoordinatesManager::getOffset()[CoordIndex::G54];   // Access a WCS
 *   CoordinatesManager::getWorkPositionCoordinates();   // Access the work position
 */

#pragma once

#include "../Core/Coordinates/Coordinates.h"
#include "../Core/Coordinates/PersistentCoordinates.h"
#include "ECoordinatesIndex.h"

class CoordinatesManager
{
public:
    static void Initialize();

    static void ResetPersistentOffsets();

    static Coordinates *getOffset(ECoordinatesIndex index);
    static Coordinates *getWorkPositionCoordinates(); // Returns the non-persistent work position

    static void UpdateWorkPositionFromSystemPosition(); // Sets work position in mm from system position (steps)

private:
    static Coordinates *fOffsets[ECoordinatesIndex::End];
    static Coordinates fWorkPositionCoordinates;     // Non-persistent work position (replaces gc_state.position[])

    static void InitializeOffset(ECoordinatesIndex index, const char *name, bool isPersistent);
};