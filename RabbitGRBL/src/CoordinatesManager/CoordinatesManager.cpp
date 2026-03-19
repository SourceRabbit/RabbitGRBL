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

// Static array holding pointers to all non-persistent named coordinates (WPos, PRB)
Coordinates *CoordinatesManager::fCoordinates[static_cast<uint8_t>(ECoordinate::Ended)];

// Static array holding pointers to all offsets (coordinate systems)
Coordinates *CoordinatesManager::fOffsets[static_cast<uint8_t>(ECoordinateOffset::End)];

void CoordinatesManager::Initialize()
{
    // Initialize non-persistent named coordinates (WPos, PRB)
    CoordinatesManager::InitializeCoordinate(ECoordinate::WPos, "WPos"); // Work Position
    CoordinatesManager::InitializeCoordinate(ECoordinate::PRB, "PRB");   // Probe Position

    // Initialize all standard GRBL work coordinate systems (G54-G59)
    // and predefined positions (G28, G30) as PersistentCoordinates,
    // so their values are automatically saved and loaded from NVS.
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G28, "G28", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G30, "G30", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G54, "G54", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G55, "G55", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G56, "G56", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G57, "G57", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G58, "G58", true);
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G59, "G59", true);

    // Non-persistent offsets (coordinate systems)
    CoordinatesManager::InitializeOffset(ECoordinateOffset::G92, "G92", false);
}

// Create a plain Coordinates object for the given ECoordinate index and name.
// Coordinates created here are always non-persistent and are never saved to NVS.
void CoordinatesManager::InitializeCoordinate(ECoordinate index, const char *name)
{
    Coordinates *coord = new Coordinates(name);
    coord->setDefault();
    fCoordinates[static_cast<uint8_t>(index)] = coord;
}

// Create a Coordinates or PersistentCoordinates object for the given index and name,
// depending on the isPersistent flag.
// If isPersistent is true, a PersistentCoordinates is created and its values are
// restored from NVS (falling back to defaults on failure).
// If isPersistent is false, a plain Coordinates is created and set to defaults.
void CoordinatesManager::InitializeOffset(ECoordinateOffset index, const char *name, bool isPersistent)
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
    fOffsets[static_cast<uint8_t>(index)] = coord;

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

// Reset all persistent offsets (coordinate systems) to their default values and notify the user
void CoordinatesManager::ResetPersistentOffsets()
{
    for (auto idx = ECoordinateOffset::Begin; idx < ECoordinateOffset::End; ++idx)
    {
        // Only reset coordinates that are persistent (PersistentCoordinates instances)
        if (CoordinatesManager::getOffset(idx)->isPersistent())
        {
            CoordinatesManager::getOffset(idx)->setDefault();
        }
    }

    MessageSender::SendMessage(EMessageLevel::Info, "Position offsets reset done");
}

// Updates the given coordinate in mm from the current system position (in steps).
// Called by the system abort and hard limit pull-off routines (WPos),
// and by Probe::StateMonitor() when the probe is triggered (PRB).
void CoordinatesManager::UpdateCoordinateFromSystemPosition(ECoordinate coordinate)
{
    float pos[MAX_N_AXIS];
    system_convert_array_steps_to_mpos(pos, sys_position);
    CoordinatesManager::getCoordinates(coordinate)->set(pos);
}

// Returns a pointer to the Coordinates object at the given offset index
Coordinates *CoordinatesManager::getOffset(ECoordinateOffset index)
{
    return fOffsets[static_cast<uint8_t>(index)];
}

// Returns a pointer to the non-persistent Coordinates object at the given coordinate index
Coordinates *CoordinatesManager::getCoordinates(ECoordinate coordinate)
{
    return fCoordinates[static_cast<uint8_t>(coordinate)];
}
