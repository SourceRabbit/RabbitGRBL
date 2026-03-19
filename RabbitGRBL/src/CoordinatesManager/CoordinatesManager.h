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
 * fCoordinates holds all non-persistent named coordinates (WPos, PRB).
 * These are indexed via the ECoordinate enum and are never saved to NVS.
 *
 * Usage:
 *   CoordinatesManager::Initialize();                                              // Call once at startup
 *   CoordinatesManager::getOffset(ECoordinateOffset::G54);                        // Access a WCS offset
 *   CoordinatesManager::getCoordinates(ECoordinate::WPos);                        // Access the work position
 *   CoordinatesManager::getCoordinates(ECoordinate::PRB);                         // Access the probe position
 *   CoordinatesManager::UpdateCoordinateFromSystemPosition(ECoordinate::WPos);    // Update work position from sys_position (steps -> mm)
 *   CoordinatesManager::UpdateCoordinateFromSystemPosition(ECoordinate::PRB);     // Update probe position from sys_position (steps -> mm)

 */

#pragma once

#include "../Core/Coordinates/Coordinates.h"
#include "../Core/Coordinates/PersistentCoordinates.h"
#include "ECoordinateOffset.h"
#include "ECoordinate.h"

class CoordinatesManager
{
public:
    static void Initialize();

    static void ResetPersistentOffsets();

    static void UpdateCoordinateFromSystemPosition(ECoordinate coordinate); // Updates the given coordinate in mm from sys_position (steps)

    static Coordinates *getOffset(ECoordinateOffset index);
    static Coordinates *getCoordinates(ECoordinate coordinate); // Returns the non-persistent coordinate at the given index



private:
    static Coordinates *fOffsets[static_cast<uint8_t>(ECoordinateOffset::End)];
    static Coordinates *fCoordinates[static_cast<uint8_t>(ECoordinate::Ended)]; // Non-persistent named coordinates (WPos, PRB)

    static void InitializeCoordinate(ECoordinate index, const char *name); // Always non-persistent
    static void InitializeOffset(ECoordinateOffset index, const char *name, bool isPersistent);
};