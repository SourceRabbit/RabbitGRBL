/*
  PersistentCoordinates.h

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

#include "Coordinates.h"

/**
 * PersistentCoordinates extends Coordinates with NVS (Non-Volatile Storage)
 * save and load functionality.
 *
 * Use this class for coordinate systems that must survive power cycles,
 * such as G54-G59 (Work Coordinate Systems) and G28/G30 (Home Positions).
 *
 * Non-persistent offsets (e.g. G92) should use the plain Coordinates class.
 */
class PersistentCoordinates : public Coordinates
{
public:
	PersistentCoordinates(const char *name) : Coordinates(name)
	{
	}

	// Load the coordinate values from NVS.
	// Returns true on success, false if the value could not be restored.
	bool LoadFromNVS();

	// Returns true because this coordinate system is persistent (saved to NVS).
	bool isPersistent() override
	{
		return true;
	}

	// Override set() to also persist the new value to NVS after updating memory.
	void set(const float *value) override;
};
