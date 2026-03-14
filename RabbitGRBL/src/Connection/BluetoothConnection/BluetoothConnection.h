/*
  BluetoothConnection.h

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
#include "BluetoothSerial.h"

class InputBuffer;

// Bluetooth Classic (SPP - Serial Port Profile) connection implementation.
// Allows external devices (e.g. phone, PC) to send G-code to RabbitGRBL
// over Bluetooth, in the same way as a wired serial connection.
class BluetoothConnection final : public Connection
{

public:
    // Constructor - accepts the Bluetooth device name
    // that will appear in the paired devices list of the connecting device.
    explicit BluetoothConnection(const char *deviceName = "RabbitGRBL");
    ~BluetoothConnection() override = default;

    void Init() override;

    int Read() override;
    bool Push(const char *data) override;

    void ResetReadBuffer();
    uint8_t GetRxBufferAvailable() override;

    size_t Write(const uint8_t *data, size_t len) override;
    size_t Write(const char *text) override;

    // Returns true if a Bluetooth client is currently connected.
    bool IsClientConnected();

private:
    // Reads one character from the BluetoothSerial stream.
    // Returns true if a byte was available and the RX buffer has space,
    // false if no data is available or the buffer is full.
    bool GetClientChar(uint8_t *data);

    // Static thunk required by FreeRTOS - casts pvParameters back to `this`
    // and delegates to ClientCheckTaskLoop().
    static void ClientCheckTaskThunk(void *pvParameters);

    // Main loop of the FreeRTOS RX task. Reads incoming Bluetooth bytes,
    // dispatches realtime commands immediately and buffers everything else.
    void ClientCheckTaskLoop();

private:
    // Bluetooth device name shown to the user during pairing.
    const char *fDeviceName;

    // Handle of the FreeRTOS RX polling task.
    TaskHandle_t fTaskHandle = nullptr;

    // Spinlock used to protect fInputBuffer from concurrent access
    // between the RX task and callers of Read() / Push() / GetRxBufferAvailable().
    portMUX_TYPE fInputBufferMutex = portMUX_INITIALIZER_UNLOCKED;

    // FreeRTOS mutex used to serialize TX writes.
    // A mutex is used instead of a critical section because BluetoothSerial.write()
    // may block and must not run inside a critical section.
    SemaphoreHandle_t fSendDataMutex = nullptr;

    // RX ring buffer for incoming data.
    // RX_BUFFER_SIZE is defined in Config.h.
    InputBuffer fInputBuffer{RX_BUFFER_SIZE};

    // Arduino BluetoothSerial object for ESP32 Classic BT (SPP).
    BluetoothSerial fSerialBT;
};