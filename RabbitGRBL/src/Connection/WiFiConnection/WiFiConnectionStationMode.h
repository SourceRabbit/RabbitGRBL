/*
  WiFiConnectionStationMode.h

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
#include <WiFi.h>

class InputBuffer;

// WiFi TCP Server connection implementation - Station Mode.
// The ESP32 connects to an existing Access Point defined by ssid/password
// and opens a TCP Server on the given port.
// An external client (e.g. PC, phone) connects to the Server and sends commands.
class WiFiConnectionStationMode final : public Connection
{

public:
    // Constructor - accepts the WiFi SSID, password and TCP Server port.
    // These values are typically defined as #define constants in the machine config file.
    explicit WiFiConnectionStationMode(const char *ssid, const char *password, uint16_t serverPort);
    ~WiFiConnectionStationMode() override = default;

    void Init() override;

    int Read() override;
    bool Push(const char *data) override;

    void ResetReadBuffer();
    uint8_t GetRxBufferAvailable() override;

    size_t Write(const uint8_t *data, size_t len) override;
    size_t Write(const char *text) override;

    // Returns true if the WiFi network connection was successfully established during Init().
    bool IsWifiConnected() const;

    // Returns true if a TCP client is currently connected.
    bool IsClientConnected();

private:
    // Reads one character from the TCP client stream.
    // Returns true if a byte was available and the RX buffer has space,
    // false if no data is available or the buffer is full.
    bool GetClientChar(uint8_t *data);

    // Static thunk required by FreeRTOS - casts pvParameters back to `this`
    // and delegates to ClientCheckTaskLoop().
    static void ClientCheckTaskThunk(void *pvParameters);

    // Main loop of the FreeRTOS RX task. Reads incoming bytes from the TCP client,
    // dispatches realtime commands immediately and buffers everything else.
    void ClientCheckTaskLoop();

    // FreeRTOS task for accepting new incoming TCP client connections.
    // Runs in a loop and accepts one client at a time.
    static void AcceptClientTaskThunk(void *pvParameters);
    void AcceptClientTaskLoop();

private:
    // WiFi credentials and TCP Server port passed via constructor.
    String fSsid;
    String fPassword;
    uint16_t fServerPort;

    // TCP Server listening for incoming connections.
    WiFiServer fServer;

    // Currently connected TCP client.
    WiFiClient fClient;

    // Handle of the FreeRTOS RX polling task.
    TaskHandle_t fRxTaskHandle = nullptr;

    // Handle of the FreeRTOS client accept task.
    TaskHandle_t fAcceptTaskHandle = nullptr;

    // Spinlock used to protect fInputBuffer from concurrent access
    // between the RX task and callers of Read() / Push() / GetRxBufferAvailable().
    portMUX_TYPE fInputBufferMutex = portMUX_INITIALIZER_UNLOCKED;

    // FreeRTOS mutex used to serialize TX writes.
    // A mutex is used instead of a critical section because WiFiClient.write()
    // may block and must not run inside a critical section.
    SemaphoreHandle_t fSendDataMutex = nullptr;

    // True after a successful WiFi network connection during Init().
    bool fWifiConnected = false;

    // RX ring buffer for incoming data.
    // RX_BUFFER_SIZE is defined in Config.h.
    InputBuffer fInputBuffer{RX_BUFFER_SIZE};
};