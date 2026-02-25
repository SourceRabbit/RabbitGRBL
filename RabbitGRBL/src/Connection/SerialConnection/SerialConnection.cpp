/*
  SerialConnection.cpp

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

#include "../../Grbl.h"
#include "SerialConnection.h"

SerialConnection::SerialConnection() = default;
enum class Cmd : uint8_t;

void SerialConnection::Init()
{
    Serial.begin(BAUD_RATE, SERIAL_8N1, 3, 1, false);
    ResetReadBuffer();
    Serial.write("\r\n"); // create some white space after ESP32 boot info

    // Create a task to check for incoming data.
    xTaskCreatePinnedToCore(SerialConnection::ClientCheckTaskThunk, "clientCheckTask", 4096, this, 1, &m_taskHandle, SUPPORT_TASK_CORE);
}

void SerialConnection::ResetReadBuffer()
{
    m_buffer.begin();
}

bool SerialConnection::GetClientChar(uint8_t *data)
{
    int res = Serial.read();
    if (res == -1)
    {
        return false;
    }

    if (m_buffer.availableforwrite())
    {
        *data = static_cast<uint8_t>(res);
        return true;
    }

    // Buffer is full - byte is intentionally discarded.
    return false;
}

void SerialConnection::ClientCheckTaskThunk(void *pvParameters)
{
    // NOTE: pvParameters is expected to be `this`.
    auto *self = static_cast<SerialConnection *>(pvParameters);
    self->ClientCheckTaskLoop();
}

void SerialConnection::ClientCheckTaskLoop()
{
    uint8_t data = 0;

    while (true)
    {
        while (GetClientChar(&data))
        {
            if (ConnectionManager::IsRealtimeCommand(data))
            {
                ConnectionManager::ExecuteRealtimeCommand(static_cast<Cmd>(data));
            }
            else
            {
                taskENTER_CRITICAL(&m_dataMutex);
                m_buffer.write(data);
                taskEXIT_CRITICAL(&m_dataMutex);
            }
        }

        // Yield CPU to allow other tasks to run.
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int SerialConnection::Read()
{
    taskENTER_CRITICAL(&m_dataMutex);
    int data = m_buffer.read();
    taskEXIT_CRITICAL(&m_dataMutex);
    return data;
}

uint8_t SerialConnection::GetRxBufferAvailable()
{
    taskENTER_CRITICAL(&m_dataMutex);
    uint8_t available = m_buffer.availableforwrite();
    taskEXIT_CRITICAL(&m_dataMutex);
    return available;
}

size_t SerialConnection::Write(const uint8_t *data, size_t len)
{
    return Serial.write(data, len);
}

size_t SerialConnection::Write(const char *text)
{
    if (text == nullptr)
    {
        return 0;
    }

    // Arduino: writes bytes until the terminating '\0'.
    return Serial.write(text);
}