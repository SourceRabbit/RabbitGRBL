/*
  Connection.h

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

#include "../Config.h"
#include <stdint.h>
#include <stddef.h>

// Generic connection interface (Serial/WiFi/Bluetooth).
class Connection
{
public:
  virtual ~Connection() = default;

  // Initializes transport and starts any background processing (task/polling).
  virtual void Init() = 0;

  // Read one byte from the internal RX buffer.
  // Returns [0..255] or -1 if no data is available.
  virtual int Read() = 0;

  // Free space in RX buffer (used for status reporting).
  virtual uint8_t GetRxBufferAvailable() = 0;

  // Write raw bytes.
  virtual size_t Write(const uint8_t *data, size_t len) = 0;

  // Write a null-terminated C string (generic alternative to Arduino String).
  virtual size_t Write(const char *text) = 0;

  // Formats a printf-style message and writes it to the connection.
  // Uses a stack buffer (TX_BUFFER_SIZE) for small messages to avoid heap
  // allocation. Falls back to heap allocation only if the formatted message
  // exceeds TX_BUFFER_SIZE. Returns the number of bytes written, or 0 on failure.
  virtual size_t WriteFormatted(const char *format, ...)
  {
    char loc_buf[TX_BUFFER_SIZE];
    char *temp = loc_buf;

    va_list arg;
    va_start(arg, format);

    // Use a copy to measure the required length,
    // keeping arg intact for the actual formatting pass.
    va_list copy;
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, copy); // measure using copy
    va_end(copy);                                  // done with copy

    if (len >= sizeof(loc_buf))
    {
      // Message is too large for the stack buffer, fall back to heap.
      temp = new char[len + 1];
      if (temp == NULL)
      {
        // Memory allocation failed, abort gracefully.
        va_end(arg);
        return 0;
      }
    }

    // arg is still intact here, safe to use for the final formatting pass.
    vsnprintf(temp, len + 1, format, arg);
    va_end(arg);

    // Write the formatted string to the connection.
    size_t written = this->Write(temp);

    // Free heap memory only if it was allocated (i.e. temp != stack buffer).
    if (temp != loc_buf)
    {
      delete[] temp;
    }

    return written;
  }

  // Inject a null-terminated G-code command string into the RX buffer,
  // as if it was received from the physical connection.
  // Returns true if the data fits and was accepted, false if the buffer is full.
  virtual bool Push(const char *data) = 0;

  virtual void ResetReadBuffer();
};