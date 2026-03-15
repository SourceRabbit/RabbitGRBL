/*
  ConnectionManager.cpp

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

#include "../Grbl.h"
#include "ConnectionManager.h"
#include "SerialConnection/SerialConnection.h"
#include "WiFiConnection/WiFiConnectionStationMode.h"
#include "WiFiConnection/WiFiConnectionAccessPointMode.h"

#ifdef ENABLE_BLUETOOTH
#include "BluetoothConnection/BluetoothConnection.h"
#endif

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------
Connection *ConnectionManager::fConnections[ConnectionManager::MAX_CONNECTIONS] = {};
int ConnectionManager::fConnectionCount = 0;

// Static storage for the concrete connection objects.
// These pointers are kept so we can manage their lifetime; they are also
// registered via AddConnection() during initialization.
static SerialConnection *fSerialConnection = nullptr;
static Connection *fWifiConnection = nullptr;

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------

void ConnectionManager::Initialize()
{
    // Reset to a known state.
    fConnectionCount = 0;
    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        fConnections[i] = nullptr;
    }

#ifdef ENABLE_BLUETOOTH
    // Bluetooth-only mode (legacy compile-time path).
    auto *btConnection = new BluetoothConnection(BT_DEVICE_NAME);
    btConnection->Init();
    AddConnection(btConnection);
#else
    // Serial is always initialized first so that there is always a working
    // communication channel, even before WiFi settings are loaded.
    fSerialConnection = new SerialConnection();
    fSerialConnection->Init();
    AddConnection(fSerialConnection);
#endif
}

void ConnectionManager::InitializeWiFi()
{
#ifdef ENABLE_BLUETOOTH
    MessageSender::SendMessage(EMessageLevel::Warning, "Cannot initialize WiFi while Bluetooth is on!");
    // Never Initialize Wifi while using Bluetooth
    return;
#endif

    // WiFi mode is controlled by setting $73:
    //   0 -> Off (default)
    //   1 -> Station mode (connect to the configured Access Point)
    //   2 -> Access Point mode (ESP32 creates its own WiFi network)
    if (settings_wifi_mode == nullptr || settings_wifi_mode->get() == 0)
    {
        return;
    }

    const char *ssid = settings_wifi_ssid ? settings_wifi_ssid->get() : "";
    if (ssid[0] == '\0')
    {
        MessageSender::SendMessage(EMessageLevel::Warning, "WiFi SSID ($74) is not set - WiFi disabled");
        return;
    }

    // Spawn WiFi initialization in its own task so it does not block system startup.
    BaseType_t result = xTaskCreatePinnedToCore(
        ConnectionManager::InitializeWiFiTask,
        "wifiInitTask",
        4096,
        nullptr,
        1,
        nullptr,
        SUPPORT_TASK_CORE);

    if (result != pdPASS)
    {
        MessageSender::SendMessage(EMessageLevel::Error, "WiFi init task creation failed");
    }
}

void ConnectionManager::InitializeWiFiTask(void *pvParameters)
{
    const char *ssid = settings_wifi_ssid ? settings_wifi_ssid->get() : "";
    const char *password = settings_wifi_password ? settings_wifi_password->get() : "";
    int wifiMode = settings_wifi_mode ? settings_wifi_mode->get() : 0;

    if (wifiMode == 2)
    {
        // Access Point mode: the ESP32 creates its own WiFi network.
        MessageSender::SendMessage(EMessageLevel::Info, "Starting WiFi Access Point: %s", ssid);

        auto *apConnection = new WiFiConnectionAccessPointMode(ssid, password, DEFAULT_WIFI_SERVER_PORT);
        apConnection->Init();
        fWifiConnection = apConnection;

        if (!apConnection->IsWifiConnected())
        {
            MessageSender::SendMessage(EMessageLevel::Error, "WiFi Access Point failed to start");
            delete fWifiConnection;
            fWifiConnection = nullptr;
        }
        else
        {
            // AP started: register it so Serial and WiFi coexist.
            AddConnection(fWifiConnection);
            MessageSender::SendMessage(EMessageLevel::Info, "WiFi Access Point started - IP: %s:%d", WiFi.softAPIP().toString().c_str(), DEFAULT_WIFI_SERVER_PORT);
        }
    }
    else
    {
        // Station mode: the ESP32 connects to an existing Access Point.
        MessageSender::SendMessage(EMessageLevel::Info, "Connecting to WiFi network: %s", ssid);

        auto *staConnection = new WiFiConnectionStationMode(ssid, password, DEFAULT_WIFI_SERVER_PORT);
        staConnection->Init();
        fWifiConnection = staConnection;

        if (!staConnection->IsWifiConnected())
        {
            MessageSender::SendMessage(EMessageLevel::Error, "WiFi connection failed");
            delete fWifiConnection;
            fWifiConnection = nullptr;
        }
        else
        {
            // WiFi connected: register it so Serial and WiFi coexist.
            AddConnection(fWifiConnection);
            MessageSender::SendMessage(EMessageLevel::Info, "WiFi connected - IP: %s:%d", WiFi.localIP().toString().c_str(), DEFAULT_WIFI_SERVER_PORT);
        }
    }

    vTaskDelete(nullptr);
}

// ---------------------------------------------------------------------------
// Connection registry
// ---------------------------------------------------------------------------

void ConnectionManager::AddConnection(Connection *conn)
{
    if (fConnectionCount < MAX_CONNECTIONS && conn != nullptr)
    {
        fConnections[fConnectionCount++] = conn;
    }
}

// ---------------------------------------------------------------------------
// I/O -- forwarded to all registered connections
// ---------------------------------------------------------------------------

int ConnectionManager::Read()
{
    // Return the first available byte from any registered connection.
    for (int i = 0; i < fConnectionCount; i++)
    {
        int data = fConnections[i]->Read();
        if (data != -1)
        {
            return data;
        }
    }
    return -1;
}

bool ConnectionManager::Push(const char *data)
{
    bool result = false;
    for (int i = 0; i < fConnectionCount; i++)
    {
        result |= fConnections[i]->Push(data);
    }
    return result;
}

uint8_t ConnectionManager::GetRxBufferAvailable()
{
    if (fConnectionCount == 0)
    {
        return 0;
    }
    uint8_t minAvailable = 255;
    for (int i = 0; i < fConnectionCount; i++)
    {
        uint8_t available = fConnections[i]->GetRxBufferAvailable();
        if (available < minAvailable)
        {
            minAvailable = available;
        }
    }
    return minAvailable;
}

size_t ConnectionManager::Write(const uint8_t *data, size_t len)
{
    // Write to all registered connections.
    // Return the byte count from the first connection (typically Serial); writes to
    // subsequent connections (e.g. WiFi when no client is connected) may silently
    // return 0 and that is not treated as an error.
    size_t written = 0;
    for (int i = 0; i < fConnectionCount; i++)
    {
        size_t n = fConnections[i]->Write(data, len);
        if (i == 0)
        {
            written = n;
        }
    }
    return written;
}

size_t ConnectionManager::Write(const char *text)
{
    size_t written = 0;
    for (int i = 0; i < fConnectionCount; i++)
    {
        size_t n = fConnections[i]->Write(text);
        if (i == 0)
        {
            written = n;
        }
    }
    return written;
}

size_t ConnectionManager::WriteFormatted(const char *format, ...)
{
    char loc_buf[TX_BUFFER_SIZE];
    char *temp = loc_buf;

    va_list arg;
    va_start(arg, format);

    va_list copy;
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, copy);
    va_end(copy);

    if (len >= sizeof(loc_buf))
    {
        temp = new char[len + 1];
        if (temp == NULL)
        {
            va_end(arg);
            return 0;
        }
    }

    vsnprintf(temp, len + 1, format, arg);
    va_end(arg);

    size_t written = ConnectionManager::Write(temp);

    if (temp != loc_buf)
    {
        delete[] temp;
    }

    return written;
}

void ConnectionManager::ResetReadBuffer()
{
    for (int i = 0; i < fConnectionCount; i++)
    {
        fConnections[i]->ResetReadBuffer();
    }
}

/**
 * TODO: Realtime commands should be moved to other manager
 */
void ConnectionManager::ExecuteRealtimeCommand(Cmd command)
{
    // GRBL realtime command execution (shared by all connection types for now).
    switch (command)
    {
    case Cmd::Reset:
        mc_reset(); // Motion control reset.
        break;

    case Cmd::StatusReport:
        report_realtime_status(); // Direct call instead of setting a flag.
        break;

    case Cmd::CycleStart:
        sys_rt_exec_state.bit.cycleStart = true;
        break;

    case Cmd::FeedHold:
        sys_rt_exec_state.bit.feedHold = true;
        break;

    case Cmd::SafetyDoor:
        sys_rt_exec_state.bit.safetyDoor = true;
        break;

    case Cmd::JogCancel:
        if (sys.state == State::Jog)
        {
            // Block all other states from invoking motion cancel.
            sys_rt_exec_state.bit.motionCancel = true;
        }
        break;

    case Cmd::SpindleOvrStop:
        sys_rt_exec_accessory_override.bit.spindleOvrStop = 1;
        break;

    case Cmd::FeedOvrReset:
        sys_rt_f_override = FeedOverride::Default;
        break;

    case Cmd::FeedOvrCoarsePlus:
        sys_rt_f_override += FeedOverride::CoarseIncrement;
        if (sys_rt_f_override > FeedOverride::Max)
        {
            sys_rt_f_override = FeedOverride::Max;
        }
        break;

    case Cmd::FeedOvrCoarseMinus:
        sys_rt_f_override -= FeedOverride::CoarseIncrement;
        if (sys_rt_f_override < FeedOverride::Min)
        {
            sys_rt_f_override = FeedOverride::Min;
        }
        break;

    case Cmd::FeedOvrFinePlus:
        sys_rt_f_override += FeedOverride::FineIncrement;
        if (sys_rt_f_override > FeedOverride::Max)
        {
            sys_rt_f_override = FeedOverride::Max;
        }
        break;

    case Cmd::FeedOvrFineMinus:
        sys_rt_f_override -= FeedOverride::FineIncrement;
        if (sys_rt_f_override < FeedOverride::Min)
        {
            sys_rt_f_override = FeedOverride::Min;
        }
        break;

    case Cmd::RapidOvrReset:
        sys_rt_r_override = RapidOverride::Default;
        break;

    case Cmd::RapidOvrMedium:
        sys_rt_r_override = RapidOverride::Medium;
        break;

    case Cmd::RapidOvrLow:
        sys_rt_r_override = RapidOverride::Low;
        break;

    case Cmd::RapidOvrExtraLow:
        sys_rt_r_override = RapidOverride::ExtraLow;
        break;

    case Cmd::SpindleOvrReset:
        sys_rt_s_override = SpindleSpeedOverride::Default;
        break;

    case Cmd::SpindleOvrCoarsePlus:
        sys_rt_s_override += SpindleSpeedOverride::CoarseIncrement;
        if (sys_rt_s_override > SpindleSpeedOverride::Max)
        {
            sys_rt_s_override = SpindleSpeedOverride::Max;
        }
        break;

    case Cmd::SpindleOvrCoarseMinus:
        sys_rt_s_override -= SpindleSpeedOverride::CoarseIncrement;
        if (sys_rt_s_override < SpindleSpeedOverride::Min)
        {
            sys_rt_s_override = SpindleSpeedOverride::Min;
        }
        break;

    case Cmd::SpindleOvrFinePlus:
        sys_rt_s_override += SpindleSpeedOverride::FineIncrement;
        if (sys_rt_s_override > SpindleSpeedOverride::Max)
        {
            sys_rt_s_override = SpindleSpeedOverride::Max;
        }
        break;

    case Cmd::SpindleOvrFineMinus:
        sys_rt_s_override -= SpindleSpeedOverride::FineIncrement;
        if (sys_rt_s_override < SpindleSpeedOverride::Min)
        {
            sys_rt_s_override = SpindleSpeedOverride::Min;
        }
        break;

    case Cmd::CoolantFloodOvrToggle:
        sys_rt_exec_accessory_override.bit.coolantFloodOvrToggle = 1;
        break;

    case Cmd::CoolantMistOvrToggle:
        sys_rt_exec_accessory_override.bit.coolantMistOvrToggle = 1;
        break;

    default:
        break;
    }
}

/**
 * TODO: Realtime commands should be moved to other manager
 */
bool ConnectionManager::IsRealtimeCommand(uint8_t data)
{
    // Extended realtime commands are >= 0x80.
    if (data >= 0x80)
    {
        return true;
    }

    // Some realtime commands are in the ASCII control range.
    auto cmd = static_cast<Cmd>(data);
    return cmd == Cmd::Reset || cmd == Cmd::StatusReport || cmd == Cmd::CycleStart || cmd == Cmd::FeedHold;
}
