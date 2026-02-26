/*
  NVSManager.h

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

/*
  NVSManager provides a centralized interface for all Non-Volatile Storage (NVS)
  operations in Rabbit GRBL. It wraps the ESP-IDF NVS API and exposes a clean,
  type-safe set of static methods for reading and writing settings to flash memory.

  All NVS access in the system must go through this class. Direct use of the
  ESP-IDF NVS API (nvs_get_*, nvs_set_*, nvs_erase_*, etc.) outside of this
  class is not allowed.

  Responsibilities:
    - Initializing the NVS flash partition on startup (Initialize)
    - Reading and writing typed values: int32, int8, float, string, blob
    - Erasing individual keys or the entire namespace
    - Reporting NVS partition usage statistics

  All settings are stored under the "Grbl_ESP32" namespace.
*/

#pragma once


#include "../Diagnostics/Errors/EError.h"
#include <nvs_flash.h>
#include <nvs.h>

class NVSManager
{
private:
    // fHandle holds the reference to the opened NVS namespace
    static nvs_handle fHandle;
    // fNamespace is the key name for the Grbl storage area
    static const char *fNamespace;

public:
    // Initialize the NVS flash and open the Grbl namespace
    static bool Initialize();

    // Get NVS usage statistics
    static esp_err_t GetStats(nvs_stats_t *stats);

    // Erase all settings stored in the current NVS namespace
    static EError EraseAll();

    static EError EraseNVSUponUserCommand(const char *value);

    // Erase a single key from NVS
    static EError EraseKey(const char *key);

    // Write a 32-bit integer to NVS
    static EError WriteInt(const char *key, int32_t value);

    // Write an 8-bit integer to NVS (used for flags and enums)
    static EError WriteInt8(const char *key, int8_t value);

    // Write a floating point value to NVS
    static EError WriteFloat(const char *key, float value);

    // Write a string to NVS
    static EError WriteString(const char *key, const char *value);

    // Write a binary blob to NVS
    static EError WriteBlob(const char *key, const void *value, size_t length);

    // Read a 32-bit integer from NVS. Returns true if successful.
    static bool ReadInt(const char *key, int32_t *value);

    // Read an 8-bit integer from NVS. Returns true if successful.
    static bool ReadInt8(const char *key, int8_t *value);

    // Read a floating point value from NVS. Returns true if successful.
    static bool ReadFloat(const char *key, float *value);

    // Read a string from NVS. Returns true if successful.
    static bool ReadString(const char *key, char *buffer, size_t *len);

    // Read a binary blob from NVS. Returns the esp_err_t result code.
    static esp_err_t ReadBlob(const char *key, void *outValue, size_t *length);
};