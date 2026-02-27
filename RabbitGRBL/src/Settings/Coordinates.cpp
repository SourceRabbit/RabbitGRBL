#include "../Grbl.h"

Coordinates *coords[CoordIndex::End];

bool Coordinates::load()
{
    size_t len;
    // Read the coordinate blob from NVS via NVSManager
    switch (NVSManager::ReadBlob(_name, _currentValue, &len))
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
    memcpy(&_currentValue, value, sizeof(_currentValue));
#ifdef FORCE_BUFFER_SYNC_DURING_NVS_WRITE
    protocol_buffer_synchronize();
#endif
    // Write the coordinate blob to NVS via NVSManager
    NVSManager::WriteBlob(_name, _currentValue, sizeof(_currentValue));
}
