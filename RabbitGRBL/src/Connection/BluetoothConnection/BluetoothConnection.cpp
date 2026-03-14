/*
  BluetoothConnection.cpp

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
#include "BluetoothConnection.h"

enum class Cmd : uint8_t;

BluetoothConnection::BluetoothConnection(const char *deviceName)
    : fDeviceName(deviceName)
{
}

void BluetoothConnection::Init()
{
    // Create a FreeRTOS mutex for TX serialization.
    fSendDataMutex = xSemaphoreCreateMutex();

    // Start Bluetooth in SPP (Serial Port Profile) mode.
    // The device will be discoverable under fDeviceName.
    fSerialBT.begin(fDeviceName);

    ResetReadBuffer();
    this->Write("\r\n"); // Create some white space after boot info.

    // Create a task to poll for incoming Bluetooth data.
    xTaskCreatePinnedToCore(
        BluetoothConnection::ClientCheckTaskThunk,
        "btClientCheckTask",
        4096,
        this,
        1,
        &fTaskHandle,
        SUPPORT_TASK_CORE);
}

void BluetoothConnection::ResetReadBuffer()
{
    fInputBuffer.begin();
}

bool BluetoothConnection::IsClientConnected()
{
    return fSerialBT.connected();
}

bool BluetoothConnection::GetClientChar(uint8_t *data)
{
    // Do nothing if no client is connected or no data is available.
    if (!fSerialBT.connected() || !fSerialBT.available())
    {
        return false;
    }

    int res = fSerialBT.read();
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

void BluetoothConnection::ClientCheckTaskThunk(void *pvParameters)
{
    // NOTE: pvParameters is expected to be `this`.
    auto *self = static_cast<BluetoothConnection *>(pvParameters);
    self->ClientCheckTaskLoop();
}

void BluetoothConnection::ClientCheckTaskLoop()
{
    uint8_t data = 0;

    while (true)
    {
        while (GetClientChar(&data))
        {
            if (ConnectionManager::IsRealtimeCommand(data))
            {
                // Dispatch realtime commands immediately, bypassing the RX buffer.
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

int BluetoothConnection::Read()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    int data = fInputBuffer.read();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return data;
}

bool BluetoothConnection::Push(const char *data)
{
    // Protect the shared input buffer from concurrent access by the client check task.
    taskENTER_CRITICAL(&fInputBufferMutex);
    bool result = fInputBuffer.push(data);
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return result;
}

uint8_t BluetoothConnection::GetRxBufferAvailable()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    uint8_t available = fInputBuffer.availableforwrite();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return available;
}

/**
 * Writes raw bytes to the Bluetooth transport in a thread-safe manner.
 *
 * This method serializes TX access using a FreeRTOS mutex because
 * BluetoothSerial.write() may block and must not be called inside a
 * critical section.
 *
 * @param data  Pointer to the bytes to write.
 * @param len   Number of bytes to write.
 * @return      The number of bytes written.
 */
size_t BluetoothConnection::Write(const uint8_t *data, size_t len)
{
    // Validate input to avoid undefined behavior and unnecessary locking.
    if (data == nullptr || len == 0)
    {
        return 0;
    }

    // Skip write if no client is connected.
    if (!fSerialBT.connected())
    {
        return 0;
    }

    // Serialize TX access across tasks.
    // NOTE: We use a FreeRTOS mutex (not a critical section) because
    // BluetoothSerial.write() may block for a non-trivial amount of time.
    if (fSendDataMutex)
    {
        // Wait indefinitely until the TX mutex becomes available.
        xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    }

    // Write raw bytes to the Bluetooth transport.
    size_t result = fSerialBT.write(data, len);

    // Release the TX mutex so other tasks can write.
    if (fSendDataMutex)
    {
        xSemaphoreGive(fSendDataMutex);
    }

    return result;
}

/**
 * Writes a null-terminated C string to the Bluetooth transport in a thread-safe manner.
 *
 * This method serializes TX access using a FreeRTOS mutex to prevent
 * interleaved output when multiple tasks write concurrently.
 *
 * @param text  Null-terminated string to write.
 * @return      The number of bytes written.
 */
size_t BluetoothConnection::Write(const char *text)
{
    // Validate input to avoid dereferencing null pointers.
    if (text == nullptr)
    {
        return 0;
    }

    // Skip write if no client is connected.
    if (!fSerialBT.connected())
    {
        return 0;
    }

    // Serialize TX access across tasks.
    if (fSendDataMutex)
    {
        // Wait indefinitely until the TX mutex becomes available.
        xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    }

    // Write the null-terminated string to the Bluetooth transport.
    size_t result = fSerialBT.write(
        reinterpret_cast<const uint8_t *>(text),
        strlen(text));

    // Release the TX mutex so other tasks can write.
    if (fSendDataMutex)
    {
        xSemaphoreGive(fSendDataMutex);
    }

    return result;
}