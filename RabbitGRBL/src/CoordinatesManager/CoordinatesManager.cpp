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

// Static array holding pointers to all coordinate systems
Coordinates *CoordinatesManager::fCoordinates[ECoordinatesIndex::End];

void CoordinatesManager::Initialize()
{
    // Initialize all standard GRBL work coordinate systems (G54-G59)
    // and predefined positions (G28, G30)
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G54, "G54");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G55, "G55");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G56, "G56");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G57, "G57");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G58, "G58");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G59, "G59");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G28, "G28");
    CoordinatesManager::InitializeCoordinate(ECoordinatesIndex::G30, "G30");
}

// Reset all coordinate systems to their default values and notify the user
void CoordinatesManager::Reset()
{
    for (auto idx = ECoordinatesIndex::Begin; idx < ECoordinatesIndex::End; ++idx)
    {
        CoordinatesManager::getCoordinates(idx)->setDefault();
    }

    MessageSender::SendMessage(EMessageLevel::Info, "Position offsets reset done");
}

// Create a Coordinates object for the given index and name,
// then attempt to restore its values from NVS.
// If NVS load fails, the coordinate is initialized to its default values.
void CoordinatesManager::InitializeCoordinate(ECoordinatesIndex index, const char *name)
{
    auto coord = new Coordinates(name);

    // Store the coordinate pointer in the static array
    fCoordinates[index] = coord;

    // Try to restore the coordinate from NVS; fall back to defaults on failure
    if (!coord->LoadFromNVS())
    {
        CoordinatesManager::getCoordinates(index)->setDefault();
    }
}

// Returns a pointer to the Coordinates object at the given index
Coordinates *CoordinatesManager::getCoordinates(ECoordinatesIndex index)
{
    return fCoordinates[index];
}