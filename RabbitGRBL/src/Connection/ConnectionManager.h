/*
  ConnectionManager.h

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

#pragma once

#include "InputBuffer/InputBuffer.h"
#include "MessageSender/MessageSender.h"
#include "Connection.h"

enum class Cmd : uint8_t;

// Manages all active connections.
// Broadcasts writes to every registered connection and muxes reads.
class ConnectionManager
{
public:
    /**
     * Initializes the connection subsystem.
     *
     * Notes:
     * - This should be called once during system startup.
     */
    static void Initialize();

    /**
     * Initializes the WiFi connection using runtime settings ($73/$74/$75).
     *
     * Notes:
     * - Must be called AFTER SettingsManager::Initialize() so that WiFi settings
     *   have been loaded from NVS.
     * - If WiFi mode ($73) is 0 or SSID ($74) is empty, this is a no-op.
     * - $73=1: Station mode - the ESP32 connects to an existing Access Point.
     * - $73=2: Access Point mode - the ESP32 creates its own WiFi network.
     * - On success the WiFi connection is added alongside Serial so that both
     *   connections coexist.
     * - The actual connection attempt runs in a dedicated FreeRTOS task so that
     *   this call returns immediately and does not block system startup.
     */
    static void InitializeWiFi();

    /**
     * Registers a connection.  Must already be initialized before being added.
     * This class does NOT take ownership of the pointer.
     * Silent no-op if the limit is exceeded or conn is null.
     */
    static void AddConnection(Connection *conn);

    // -------------------------------------------------------------------------
    // I/O – every operation is forwarded to all registered connections.
    // -------------------------------------------------------------------------

    /** Returns the first available byte from any registered connection, or -1. */
    static int Read();

    /** Pushes data into the RX buffer of all registered connections. */
    static bool Push(const char *data);

    /** Returns the minimum available RX buffer space across all connections. */
    static uint8_t GetRxBufferAvailable();

    /** Writes raw bytes to all registered connections.
     *  Returns the byte count from the first connection (typically Serial). */
    static size_t Write(const uint8_t *data, size_t len);

    /** Writes a null-terminated string to all registered connections.
     *  Returns the byte count from the first connection (typically Serial). */
    static size_t Write(const char *text);

    /** Formats a printf-style message and writes it to all registered connections.
     *  Returns the byte count from the first connection (typically Serial). */
    static size_t WriteFormatted(const char *format, ...);

    /** Resets the RX buffer of all registered connections. */
    static void ResetReadBuffer();

    // -------------------------------------------------------------------------

    /**
     * Executes a GRBL realtime command.
     *
     * Notes:
     * - Kept here temporarily to share the same realtime logic across connection types.
     * - In the future, this may be moved to a dedicated module (e.g. RealtimeCommands)
     *   or handled by each connection implementation.
     */
    static void ExecuteRealtimeCommand(Cmd command);

    /**
     * Returns true if the given byte is a GRBL realtime command byte.
     *
     * Notes:
     * - GRBL realtime commands are either extended (>= 0x80) or specific control bytes.
     * - Kept here temporarily to share logic across connection types.
     */
    static bool IsRealtimeCommand(uint8_t data);

private:
    // 4 slots covers the practical maximum: Serial + WiFi + Bluetooth + one spare.
    // AddConnection silently ignores any attempt to exceed this limit.
    static const int MAX_CONNECTIONS = 4;
    static Connection *fConnections[MAX_CONNECTIONS];
    static int fConnectionCount;

    // FreeRTOS task entry point for InitializeWiFi().
    // Performs the actual WiFi connection attempt in the background.
    static void InitializeWiFiTask(void *pvParameters);
};