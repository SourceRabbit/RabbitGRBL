/*
  WiFiConnectionStationMode.cpp

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
#include "WiFiConnectionStationMode.h"

enum class Cmd : uint8_t;

WiFiConnectionStationMode::WiFiConnectionStationMode(const char *ssid, const char *password, uint16_t serverPort)
    : fSsid(ssid), fPassword(password), fServerPort(serverPort), fServer(serverPort)
{
}

void WiFiConnectionStationMode::Init()
{
    // Create a FreeRTOS mutex for TX serialization.
    fSendDataMutex = xSemaphoreCreateMutex();

    // Connect to the WiFi Access Point using the provided SSID and password.
    WiFi.mode(WIFI_STA);
    WiFi.begin(fSsid.c_str(), fPassword.c_str());

    // Wait up to 30 seconds for the connection to be established (60 x 500 ms).
    const int maxAttempts = 60;
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        attempts++;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        // WiFi connection failed within the timeout - do not start the TCP server.
        return;
    }

    fWifiConnected = true;

    // Start the TCP Server on the configured port.
    fServer.begin();

    ResetReadBuffer();
    this->Write("\r\n"); // Create some white space after boot info.

    // Create a task for accepting incoming TCP client connections.
    xTaskCreatePinnedToCore(
        WiFiConnectionStationMode::AcceptClientTaskThunk,
        "wifiAcceptTask",
        4096,
        this,
        1,
        &fAcceptTaskHandle,
        SUPPORT_TASK_CORE);

    // Create a task to poll for incoming data from the TCP client.
    xTaskCreatePinnedToCore(
        WiFiConnectionStationMode::ClientCheckTaskThunk,
        "wifiClientCheckTask",
        4096,
        this,
        1,
        &fRxTaskHandle,
        SUPPORT_TASK_CORE);
}

void WiFiConnectionStationMode::ResetReadBuffer()
{
    fInputBuffer.begin();
}

bool WiFiConnectionStationMode::IsWifiConnected() const
{
    return fWifiConnected;
}

bool WiFiConnectionStationMode::IsClientConnected()
{
    return fClient && fClient.connected();
}

// --- AcceptClient Task ---

void WiFiConnectionStationMode::AcceptClientTaskThunk(void *pvParameters)
{
    // NOTE: pvParameters is expected to be `this`.
    auto *self = static_cast<WiFiConnectionStationMode *>(pvParameters);
    self->AcceptClientTaskLoop();
}

void WiFiConnectionStationMode::AcceptClientTaskLoop()
{
    while (true)
    {
        // If no client is currently connected, wait for a new one.
        if (!IsClientConnected())
        {
            WiFiClient newClient = fServer.accept();
            if (newClient)
            {
                // Store the new client and reset the RX buffer.
                fClient = newClient;
                ResetReadBuffer();
            }
        }

        // Yield CPU to allow other tasks to run.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

// --- RX Task ---

bool WiFiConnectionStationMode::GetClientChar(uint8_t *data)
{
    // Do nothing if no client is connected or no data is available.
    if (!IsClientConnected() || !fClient.available())
    {
        return false;
    }

    int res = fClient.read();
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

void WiFiConnectionStationMode::ClientCheckTaskThunk(void *pvParameters)
{
    // NOTE: pvParameters is expected to be `this`.
    auto *self = static_cast<WiFiConnectionStationMode *>(pvParameters);
    self->ClientCheckTaskLoop();
}

void WiFiConnectionStationMode::ClientCheckTaskLoop()
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

// --- Read / Push / GetRxBufferAvailable ---

int WiFiConnectionStationMode::Read()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    int data = fInputBuffer.read();
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return data;
}

bool WiFiConnectionStationMode::Push(const char *data)
{
    // Protect the shared input buffer from concurrent access by the client check task.
    taskENTER_CRITICAL(&fInputBufferMutex);
    bool result = fInputBuffer.push(data);
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return result;
}

uint8_t WiFiConnectionStationMode::GetRxBufferAvailable()
{
    taskENTER_CRITICAL(&fInputBufferMutex);
    uint8_t available = static_cast<uint8_t>(fInputBuffer.availableforwrite());
    taskEXIT_CRITICAL(&fInputBufferMutex);
    return available;
}

// --- Write ---

size_t WiFiConnectionStationMode::Write(const uint8_t *data, size_t len)
{
    if (!IsClientConnected())
    {
        return 0;
    }

    // Use mutex to prevent concurrent TX writes from different tasks.
    xSemaphoreTake(fSendDataMutex, portMAX_DELAY);
    size_t written = fClient.write(data, len);
    xSemaphoreGive(fSendDataMutex);
    return written;
}

size_t WiFiConnectionStationMode::Write(const char *text)
{
    return Write(reinterpret_cast<const uint8_t *>(text), strlen(text));
}