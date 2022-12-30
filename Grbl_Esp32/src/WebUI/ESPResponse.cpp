/*
  ESPResponse.cpp - GRBL_ESP response class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "../Grbl.h"
#include "ESPResponse.h"

namespace WebUI
{

    ESPResponseStream::ESPResponseStream()
    {
        _client = CLIENT_INPUT;
    }

    ESPResponseStream::ESPResponseStream(uint8_t client, bool byid)
    {
        (void)byid; // fake parameter to avoid confusion with pointer one (NULL == 0)
        _client = client;
    }

    void ESPResponseStream::println(const char *data)
    {
        print(data);
        if (_client == CLIENT_TELNET)
        {
            print("\r\n");
        }
        else
        {
            print("\n");
        }
    }

    // helper to format size to readable string
    String ESPResponseStream::formatBytes(uint64_t bytes)
    {
        if (bytes < 1024)
        {
            return String((uint16_t)bytes) + " B";
        }
        else if (bytes < (1024 * 1024))
        {
            return String((float)(bytes / 1024.0), 2) + " KB";
        }
        else if (bytes < (1024 * 1024 * 1024))
        {
            return String((float)(bytes / 1024.0 / 1024.0), 2) + " MB";
        }
        else
        {
            return String((float)(bytes / 1024.0 / 1024.0 / 1024.0), 2) + " GB";
        }
    }

    void ESPResponseStream::print(const char *data)
    {
        if (_client == CLIENT_INPUT)
        {
            return;
        }

        grbl_send(_client, data);
    }

    void ESPResponseStream::flush()
    {
    }
}
