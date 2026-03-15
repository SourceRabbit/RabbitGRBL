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

#ifdef ENABLE_BLUETOOTH
#include "BluetoothConnection/BluetoothConnection.h"
#endif

// Default to null; must be set during init.
Connection *ConnectionManager::fActiveConnectionPointer = nullptr;

void ConnectionManager::Initialize()
{
    // Reset to a known state during startup.
    fActiveConnectionPointer = nullptr;

#ifdef ENABLE_BLUETOOTH
    // Bluetooth-only mode: create and initialize the Bluetooth connection.
    // The device name is defined by BT_DEVICE_NAME in the machine config file.
    auto *btConnection = new BluetoothConnection(BT_DEVICE_NAME);
    btConnection->Init();
    SetActive(btConnection);
#else
    // Serial-only mode: create and initialize the Serial connection.
    auto *serialConnection = new SerialConnection();
    serialConnection->Init();
    SetActive(serialConnection);
#endif
}

void ConnectionManager::SetActive(Connection *connection)
{
    // Non-owning pointer: lifetime is managed by the caller.
    fActiveConnectionPointer = connection;
}

Connection &ConnectionManager::Active()
{
    return *fActiveConnectionPointer;
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