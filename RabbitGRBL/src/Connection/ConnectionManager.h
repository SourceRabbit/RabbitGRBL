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

#include "Connection.h"

enum class Cmd : uint8_t;

// Holds the currently active connection (Serial now, WiFi/BT later).
class ConnectionManager
{
public:
    /**
     * Sets the active connection used by the system.
     *
     * Notes:
     * - This class does NOT take ownership of the pointer.
     * - The caller is responsible for ensuring the provided object remains valid
     *   for as long as it is set as active.
     */
    static void SetActive(Connection *connection);

    /**
     * Returns a reference to the currently active connection.
     *
     * Preconditions:
     * - An active connection must have been set via SetActive() before calling this.
     *
     * Notes:
     * - This function dereferences the stored pointer, so calling it while no
     *   connection is set will result in undefined behavior (null dereference).
     */
    static Connection &Active();

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
    // Non-owning pointer to the active connection instance.
    static Connection *s_active;
};