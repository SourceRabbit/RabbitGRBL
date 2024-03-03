/*
  Serial.cpp - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

    2018 -	Bart Dring This file was modified for use on the ESP32
                    CPU. Do not use this with Grbl for atMega328P

    2024 - Nikos Siatras (https://github.com/nsiatras)

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "Grbl.h"

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

static TaskHandle_t clientCheckTaskHandle = 0;

WebUI::InputBuffer client_buffer; // create a buffer for each client

// Returns the number of bytes available in a client buffer.
uint8_t client_get_rx_buffer_available()
{
    return 128 - Serial.available();
    //    return client_buffer[client].availableforwrite();
}

void heapCheckTask(void *pvParameters)
{
    static uint32_t heapSize = 0;
    while (true)
    {
        uint32_t newHeapSize = xPortGetFreeHeapSize();
        if (newHeapSize != heapSize)
        {
            heapSize = newHeapSize;
            grbl_msg_sendf(MsgLevel::Info, "heap %d", heapSize);
        }

        vTaskDelay(3000 / portTICK_RATE_MS); // Yield to other tasks

        static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#endif
    }
}

void client_init()
{
#ifdef DEBUG_REPORT_HEAP_SIZE
    // For a 2000-word stack, uxTaskGetStackHighWaterMark reports 288 words available
    xTaskCreatePinnedToCore(heapCheckTask, "heapTask", 2000, NULL, 1, NULL, 1);
#endif

    Serial.begin(BAUD_RATE, SERIAL_8N1, 3, 1, false);
    client_reset_read_buffer();
    Serial.write("\r\n"); // create some white space after ESP32 boot info

    clientCheckTaskHandle = 0;
    // create a task to check for incoming data
    // For a 4096-word stack, uxTaskGetStackHighWaterMark reports 244 words available
    // after WebUI attaches.
    xTaskCreatePinnedToCore(clientCheckTask,   // task
                            "clientCheckTask", // name for task
                            4096,              // size of task stack
                            NULL,              // parameters
                            1,                 // priority
                            &clientCheckTaskHandle,
                            SUPPORT_TASK_CORE // must run the task on same core
                                              // core
    );
}

static uint8_t getClientChar(uint8_t *data)
{
    int res;

    if (client_buffer.availableforwrite() && (res = Serial.read()) != -1)
    {
        *data = res;
        return CLIENT_SERIAL;
    }

    return CLIENT_ALL;
}

// this task runs and checks for data on all interfaces
// REaltime stuff is acted upon, then characters are added to the appropriate buffer
void clientCheckTask(void *pvParameters)
{
    uint8_t data = 0;
    uint8_t client; // who sent the data
    static UBaseType_t uxHighWaterMark = 0;

    while (true)
    {
        // run continuously
        while ((client = getClientChar(&data)) == CLIENT_SERIAL)
        {
            // Pick off realtime command characters directly from the serial stream. These characters are
            // not passed into the main buffer, but these set system state flag bits for realtime execution.
            if (is_realtime_command(data))
            {
                execute_realtime_command(static_cast<Cmd>(data));
            }
            else
            {
                vPortEnterCritical(&myMutex);
                client_buffer.write(data);
                vPortExitCritical(&myMutex);
            }
        } // if something available

        vTaskDelay(1 / portTICK_RATE_MS); // Yield to other tasks

        static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#endif
    }
}

void client_reset_read_buffer()
{
    client_buffer.begin();
}

// Fetches the first byte in the client read buffer. Called by protocol loop.
int client_read()
{
    vPortEnterCritical(&myMutex);
    int data = client_buffer.read();
    vPortExitCritical(&myMutex);
    return data;
}

// checks to see if a character is a realtime character
bool is_realtime_command(uint8_t data)
{
    if (data >= 0x80)
    {
        return true;
    }
    auto cmd = static_cast<Cmd>(data);
    return cmd == Cmd::Reset || cmd == Cmd::StatusReport || cmd == Cmd::CycleStart || cmd == Cmd::FeedHold;
}

// Act upon a realtime character
void execute_realtime_command(Cmd command)
{
    switch (command)
    {

    case Cmd::Reset:
        // grbl_msg_sendf(MsgLevel::Debug, "Cmd::Reset");
        mc_reset(); // Call motion control reset routine.
        break;

    case Cmd::StatusReport:
        report_realtime_status(); // direct call instead of setting flag
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

    case Cmd::DebugReport:
#ifdef DEBUG
        sys_rt_exec_debug = true;
#endif
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
    }
}

void client_write(const char *text)
{
    Serial.write(text);
}