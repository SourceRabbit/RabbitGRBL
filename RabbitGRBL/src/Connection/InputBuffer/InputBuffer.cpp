/*
  InputBuffer.cpp

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

#include "../../Config.h"
#include "InputBuffer.h"

InputBuffer::InputBuffer(uint16_t bufferSize)
    : _RXbufferCap(bufferSize),
      _RXbufferSize(0),
      _RXbufferpos(0)
{
    // Allocate the ring buffer on the heap with the requested capacity.
    _RXbuffer = new uint8_t[_RXbufferCap];
}

InputBuffer::~InputBuffer()
{
    delete[] _RXbuffer;
}

void InputBuffer::begin()
{
    _RXbufferSize = 0;
    _RXbufferpos = 0;
}

void InputBuffer::end()
{
    _RXbufferSize = 0;
    _RXbufferpos = 0;
}

InputBuffer::operator bool() const { return true; }

int InputBuffer::available() { return _RXbufferSize; }

int InputBuffer::availableforwrite() { return _RXbufferCap - _RXbufferSize; }

size_t InputBuffer::write(uint8_t c)
{
    if (_RXbufferSize >= _RXbufferCap)
    {
        // Buffer is full - byte is discarded.
        return 0;
    }

    // Calculate the write position using ring-buffer arithmetic.
    uint16_t current = (_RXbufferpos + _RXbufferSize) % _RXbufferCap;
    _RXbuffer[current] = c;
    _RXbufferSize++;
    return 1;
}

size_t InputBuffer::write(const uint8_t *buffer, size_t size)
{
    // No need currently
    // keep for compatibility
    return size;
}

int InputBuffer::peek(void)
{
    if (_RXbufferSize > 0)
    {
        return _RXbuffer[_RXbufferpos];
    }

    return -1;
}

bool InputBuffer::push(const char *data)
{
    uint16_t data_size = (uint16_t)strlen(data);

    if ((data_size + _RXbufferSize) > _RXbufferCap)
    {
        // Not enough space - reject the entire string to avoid partial commands.
        return false;
    }

    for (uint16_t i = 0; i < data_size; i++)
    {
        // Calculate the write position using ring-buffer arithmetic.
        uint16_t current = (_RXbufferpos + _RXbufferSize) % _RXbufferCap;
        _RXbuffer[current] = (uint8_t)data[i];
        _RXbufferSize++;
    }

    return true;
}

int InputBuffer::read(void)
{
    if (_RXbufferSize == 0)
    {
        return -1;
    }

    int v = _RXbuffer[_RXbufferpos];

    // Advance the read position using ring-buffer arithmetic.
    _RXbufferpos = (_RXbufferpos + 1) % _RXbufferCap;
    _RXbufferSize--;
    return v;
}

void InputBuffer::flush(void)
{
    // No need currently...
}