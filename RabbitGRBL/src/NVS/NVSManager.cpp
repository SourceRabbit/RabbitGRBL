/*
  NVSManager.cpp

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
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

#include "NVSManager.h"

// Static field initialization
nvs_handle NVSManager::fHandle = 0;
const char *NVSManager::fNamespace = "Ra_GRBL";

bool NVSManager::Initialize()
{
    // Guard: if already initialized, skip re-initialization
    if (fHandle != 0)
    {
        return true;
    }

    // Initialize the default NVS partition
    esp_err_t err = nvs_flash_init();

    // If no free pages or new version found, erase and re-initialize
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }

    if (err != ESP_OK)
    {
        return false;
    }

    // Open the NVS namespace in read/write mode
    err = nvs_open(fNamespace, NVS_READWRITE, &fHandle);
    return (err == ESP_OK);
}

esp_err_t NVSManager::GetStats(nvs_stats_t *stats)
{
    // Retrieve NVS partition usage statistics
    return nvs_get_stats(NULL, stats);
}

EError NVSManager::EraseAll()
{
    if (nvs_erase_all(fHandle) == ESP_OK)
    {
        // Flush changes to the physical flash memory
        nvs_commit(fHandle);
        return EError::Ok;
    }
    return EError::NvsSetFailed;
}

EError NVSManager::EraseNVSUponUserCommand(const char *value)
{
    // Delegate erase to NVSManager
    return NVSManager::EraseAll();
}

EError NVSManager::EraseKey(const char *key)
{
    // Erase a single key-value pair from NVS and check the result
    if (nvs_erase_key(fHandle, key) == ESP_OK)
    {
        return EError::Ok;
    }
    return EError::NvsSetFailed;
}

EError NVSManager::WriteInt(const char *key, int32_t value)
{
    if (nvs_set_i32(fHandle, key, value) == ESP_OK)
    {
        nvs_commit(fHandle);
        return EError::Ok;
    }
    return EError::NvsSetFailed;
}

EError NVSManager::WriteInt8(const char *key, int8_t value)
{
    if (nvs_set_i8(fHandle, key, value) == ESP_OK)
    {
        nvs_commit(fHandle);
        return EError::Ok;
    }
    return EError::NvsSetFailed;
}

EError NVSManager::WriteFloat(const char *key, float value)
{
    // Floats are converted to bit-equivalent integers for storage
    union
    {
        int32_t ival;
        float fval;
    } v;

    v.fval = value;
    return WriteInt(key, v.ival);
}

EError NVSManager::WriteString(const char *key, const char *value)
{
    if (nvs_set_str(fHandle, key, value) == ESP_OK)
    {
        nvs_commit(fHandle);
        return EError::Ok;
    }
    return EError::NvsSetFailed;
}

EError NVSManager::WriteBlob(const char *key, const void *value, size_t length)
{
    if (nvs_set_blob(fHandle, key, value, length) == ESP_OK)
    {
        nvs_commit(fHandle);
        return EError::Ok;
    }
    return EError::NvsSetFailed;
}

bool NVSManager::ReadInt(const char *key, int32_t *value)
{
    return (nvs_get_i32(fHandle, key, value) == ESP_OK);
}

bool NVSManager::ReadInt8(const char *key, int8_t *value)
{
    return (nvs_get_i8(fHandle, key, value) == ESP_OK);
}

bool NVSManager::ReadFloat(const char *key, float *value)
{
    int32_t temp;
    if (nvs_get_i32(fHandle, key, &temp) == ESP_OK)
    {
        union
        {
            int32_t ival;
            float fval;
        } v;

        v.ival = temp;
        *value = v.fval;
        return true;
    }
    return false;
}

bool NVSManager::ReadString(const char *key, char *buffer, size_t *len)
{
    return (nvs_get_str(fHandle, key, buffer, len) == ESP_OK);
}

esp_err_t NVSManager::ReadBlob(const char *key, void *outValue, size_t *length)
{
    // Returns the raw esp_err_t so callers can handle specific error codes
    return nvs_get_blob(fHandle, key, outValue, length);
}