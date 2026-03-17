/*
  InputBuffer.h

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

#include <Print.h>
#include <cstring>

class InputBuffer : public Print
{
public:
    // Allocates an RX buffer of the given size (in bytes).
    explicit InputBuffer(uint16_t bufferSize);

    size_t write(uint8_t c);
    size_t write(const uint8_t *buffer, size_t size);
    inline size_t write(const char *s) { return write((uint8_t *)s, ::strlen(s)); }
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    void begin();
    void end();
    int available();
    int availableforwrite();
    int peek(void);
    int read(void);
    bool push(const char *data);
    void flush(void);

    operator bool() const;

    ~InputBuffer();

private:
    uint8_t *_RXbuffer;     // Heap-allocated RX ring buffer
    uint16_t _RXbufferCap;  // Total capacity of the buffer (bytes)
    uint16_t _RXbufferSize; // Number of bytes currently in the buffer
    uint16_t _RXbufferpos;  // Read position (head of the ring buffer)
};