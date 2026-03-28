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

enum class Cmd : uint8_t;

SerialConnection::SerialConnection()
{
}

void SerialConnection::Init()
{
    // Create a FreeRTOS mutex for TX serialization.
    fSendDataMutex = xSemaphoreCreateMutex();

    Serial.begin(BAUD_RATE, SERIAL_8N1, 3, 1, false);

    // Increase the ESP32 UART hardware RX buffer.
    // 512 bytes is enough for several GCode lines while
    // conserving RAM compared to a larger buffer.
    Serial.setRxBufferSize(512);

    ResetReadBuffer();
    this->Write("\r\n"); // Create some white space after ESP32 boot info

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

    // Buffer is full — byte is intentionally discarded.
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
        // Read all available bytes without delay.
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

        // Yield CPU to other tasks.
        // Using taskYIELD() instead of vTaskDelay(1ms) so we return
        // immediately when no other task of the same priority is ready —
        // this reduces the average read latency from ~1ms to a few
        // microseconds.
        taskYIELD();
    }
}

int SerialConnection::Read()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    int data = fInputBuffer.read();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return data;
}

bool SerialConnection::Push(const char *data)
{
    // Protect the shared input buffer from concurrent access by the client check task.
    taskENTER_CRITICAL(&fInputBufferMutex);
    bool result = fInputBuffer.push(data);
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return result;
}

uint8_t SerialConnection::GetRxBufferAvailable()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    uint8_t available = fInputBuffer.availableforwrite();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return available;
}

/**
 * Internal method that writes raw bytes without acquiring the mutex.
 * Must only be called from the public Write() methods that have
 * already locked fSendDataMutex.
 *
 * @param data Pointer to the bytes to write.
 * @param len  Number of bytes to write.
 * @return The number of bytes written.
 */
size_t SerialConnection::WriteRaw(const uint8_t *data, size_t len)
{
    return Serial.write(data, len);
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
    // NOTE: We use a FreeRTOS mutex (not a critical section) because
    // Serial.write() may block and can take a non-trivial amount of time.
    if (fSendDataMutex)
    {
        xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    }

    size_t result = WriteRaw(data, len);

    if (fSendDataMutex)
    {
        xSemaphoreGive(fSendDataMutex);
    }

    return result;
}

/**
 * Writes a null-terminated C string to the Serial transport in a thread-safe manner.
 *
 * Instead of calling Serial.write(text) directly, this method computes the
 * length once and delegates to WriteRaw(), avoiding an internal strlen
 * inside the Arduino framework.
 *
 * @param text Null-terminated string to write.
 * @return The number of bytes written.
 */
size_t SerialConnection::Write(const char *text)
{
    // Validate input to avoid dereferencing null pointers.
    if (text == nullptr)
    {
        return 0;
    }

    size_t len = strlen(text);
    if (len == 0)
    {
        return 0;
    }

    // Serialize TX access across tasks.
    if (fSendDataMutex)
    {
        xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    }

    // Call WriteRaw with a known length — avoids a second strlen internally.
    size_t result = WriteRaw(reinterpret_cast<const uint8_t *>(text), len);

    if (fSendDataMutex)
    {
        xSemaphoreGive(fSendDataMutex);
    }

    return result;
}