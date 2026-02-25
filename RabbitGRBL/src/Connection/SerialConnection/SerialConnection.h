/*
  SerialConnection.h

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

#include "../Connection.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class InputBuffer;

// Serial-based connection implementation.
class SerialConnection final : public Connection
{

public:
    SerialConnection();
    ~SerialConnection() override = default;

    void Init() override;

    int Read() override;
    bool Push(const char *data) override;

    void ResetReadBuffer();
    uint8_t GetRxBufferAvailable() override;

    size_t Write(const uint8_t *data, size_t len) override;
    size_t Write(const char *text) override;

private:
    bool GetClientChar(uint8_t *data);

    static void ClientCheckTaskThunk(void *pvParameters);
    void ClientCheckTaskLoop();

private:
    TaskHandle_t fTaskHandle = nullptr;
    portMUX_TYPE fInputBufferMutex = portMUX_INITIALIZER_UNLOCKED;
    SemaphoreHandle_t fSendDataMutex = nullptr; // TX mutex for Serial.write

    InputBuffer fInputBuffer;
};