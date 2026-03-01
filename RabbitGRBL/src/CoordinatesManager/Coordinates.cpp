/*
  Coordinates.cpp

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

bool Coordinates::LoadFromNVS()
{
    size_t len;
    // Read the coordinate blob from NVS via NVSManager
    switch (NVSManager::ReadBlob(fName, fCurrentValue, &len))
    {
    case ESP_OK:
        return true;
    case ESP_ERR_NVS_INVALID_LENGTH:
        // The stored value may be longer than the buffer if MAX_N_AXIS decreased.
        // Accept the initial coordinates and ignore the residue.
        return true;
    case ESP_ERR_NVS_INVALID_NAME:
    case ESP_ERR_NVS_INVALID_HANDLE:
    default:
        return false;
    }
};

void Coordinates::set(float value[MAX_N_AXIS])
{
    memcpy(&fCurrentValue, value, sizeof(fCurrentValue));
#ifdef FORCE_BUFFER_SYNC_DURING_NVS_WRITE
    protocol_buffer_synchronize();
#endif
    // Write the coordinate blob to NVS via NVSManager
    NVSManager::WriteBlob(fName, fCurrentValue, sizeof(fCurrentValue));
}
