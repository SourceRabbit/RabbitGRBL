/*
  CoordinatesManager.cpp

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

#include "../Grbl.h"

// Static array holding pointers to all offets (coordinate systems)
Coordinates *CoordinatesManager::fOffsets[ECoordinatesIndex::End];

// Non-persistent work position - initialized inline with name "WPos"
Coordinates CoordinatesManager::fWorkPositionCoordinates("WPos");

void CoordinatesManager::Initialize()
{
    // Initialize all standard GRBL work coordinate systems (G54-G59)
    // and predefined positions (G28, G30) as PersistentCoordinates,
    // so their values are automatically saved and loaded from NVS.
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G28, "G28", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G30, "G30", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G54, "G54", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G55, "G55", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G56, "G56", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G57, "G57", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G58, "G58", true);
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G59, "G59", true);

    // Non persistent offets (coordinate systems)
    CoordinatesManager::InitializeOffset(ECoordinatesIndex::G92, "G92", false);

    // Initialize work position to defaults (all zeros)
    // This is non-persistent and never saved to NVS
    fWorkPositionCoordinates.setDefault();
}

// Reset all persistent offsets (coordinate systems) to their default values and notify the user
void CoordinatesManager::ResetPersistentOffsets()
{
    for (auto idx = ECoordinatesIndex::Begin; idx < ECoordinatesIndex::End; ++idx)
    {
        // Only reset coordinates that are persistent (PersistentCoordinates instances)
        if (CoordinatesManager::getOffset(idx)->isPersistent())
        {
            CoordinatesManager::getOffset(idx)->setDefault();
        }
    }

    MessageSender::SendMessage(EMessageLevel::Info, "Position offsets reset done");
}

// Create a Coordinates or PersistentCoordinates object for the given index and name,
// depending on the isPersistent flag.
// If isPersistent is true, a PersistentCoordinates is created and its values are
// restored from NVS (falling back to defaults on failure).
// If isPersistent is false, a plain Coordinates is created and set to defaults.
void CoordinatesManager::InitializeOffset(ECoordinatesIndex index, const char *name, bool isPersistent)
{
    Coordinates *coord;

    if (isPersistent)
    {
        coord = new PersistentCoordinates(name);
    }
    else
    {
        coord = new Coordinates(name);
    }

    // Store the coordinate pointer in the static array
    fOffsets[index] = coord;

    // For persistent coordinates, try to restore from NVS; fall back to defaults on failure.
    // For non-persistent coordinates, always initialize to defaults.
    if (isPersistent)
    {
        if (!static_cast<PersistentCoordinates *>(coord)->LoadFromNVS())
        {
            coord->setDefault();
        }
    }
    else
    {
        coord->setDefault();
    }
}

// Returns a pointer to the Coordinates object at the given index
Coordinates *CoordinatesManager::getOffset(ECoordinatesIndex index)
{
    return fOffsets[index];
}

// Returns the non-persistent work position coordinates
Coordinates *CoordinatesManager::getWorkPositionCoordinates()
{
    return &fWorkPositionCoordinates;
}

// Sets g-code parser position in mm from system position in steps.
// Called by the system abort and hard limit pull-off routines.
void CoordinatesManager::UpdateWorkPositionFromSystemPosition()
{
    float pos[MAX_N_AXIS];
    system_convert_array_steps_to_mpos(pos, sys_position);
    CoordinatesManager::getWorkPositionCoordinates()->set(pos);
}