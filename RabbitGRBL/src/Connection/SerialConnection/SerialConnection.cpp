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
    // Create a FreeRTOS mutex for TX serialization.
    fSendDataMutex = xSemaphoreCreateMutex();

    Serial.begin(BAUD_RATE, SERIAL_8N1, 3, 1, false);
    ResetReadBuffer();
    this->Write("\r\n"); // create some white space after ESP32 boot info

    // Create a task to check for incoming data.
    xTaskCreatePinnedToCore(SerialConnection::ClientCheckTaskThunk, "clientCheckTask", 4096, this, 1, &fTaskHandle, SUPPORT_TASK_CORE);
}

void SerialConnection::ResetReadBuffer()
{
    fInputBuffer.begin();
}

bool SerialConnection::GetClientChar(uint8_t *data)
{
    int res = Serial.read();
    if (res == -1)
    {
        return false;
    }

    if (fInputBuffer.availableforwrite())
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
                taskENTER_CRITICAL(&fInputBufferMutex);
                fInputBuffer.write(data);
                taskEXIT_CRITICAL(&fInputBufferMutex);
            }
        }

        // Yield CPU to allow other tasks to run.
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

int SerialConnection::Read()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    int data = fInputBuffer.read();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return data;
}

uint8_t SerialConnection::GetRxBufferAvailable()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    uint8_t available = fInputBuffer.availableforwrite();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return available;
}

/**
 * Writes raw bytes to the Serial transport in a thread-safe manner.
 *
 * This method serializes TX access using a FreeRTOS mutex because Serial.write()
 * may block and must not be protected by a critical section.
 *
 * @param data Pointer to the bytes to write.
 * @param len  Number of bytes to write.
 * @return The number of bytes written (as reported by Serial.write()).
 */
size_t SerialConnection::Write(const uint8_t *data, size_t len)
{
    // Validate input to avoid undefined behavior and unnecessary locking.
    if (data == nullptr || len == 0)
    {
        return 0;
    }

    // Serialize TX access across tasks.
    // NOTE: We use a FreeRTOS mutex (not a critical section) because Serial.write()
    // may block and can take a non-trivial amount of time.
    if (fSendDataMutex)
    {
        // Wait indefinitely until the TX mutex becomes available.
        xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    }

    // Write raw bytes to the Arduino Serial transport.
    size_t result = Serial.write(data, len);

    // Release the TX mutex so other tasks can write.
    if (fSendDataMutex)
    {
        xSemaphoreGive(fSendDataMutex);
    }

    return result;
}

/**
 * Writes a null-terminated C string to the Serial transport in a thread-safe manner.
 *
 * This method serializes TX access using a FreeRTOS mutex to prevent interleaved
 * output when multiple tasks write concurrently.
 *
 * @param text Null-terminated string to write.
 * @return The number of bytes written (as reported by Serial.write()).
 */
size_t SerialConnection::Write(const char *text)
{
    // Validate input to avoid dereferencing null pointers.
    if (text == nullptr)
    {
        return 0;
    }

    // Serialize TX access across tasks.
    if (fSendDataMutex)
    {
        // Wait indefinitely until the TX mutex becomes available.
        xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    }

    // Write a null-terminated string to the Arduino Serial transport.
    size_t result = Serial.write(text);

    // Release the TX mutex so other tasks can write.
    if (fSendDataMutex)
    {
        xSemaphoreGive(fSendDataMutex);
    }

    return result;
}