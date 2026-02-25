/*
  MessageSender.cpp

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
#include "MessageSender.h"

/**
 * Sends a [MSG:...] style message to the GRBL output.
 * Messages are suppressed if their level exceeds the currently configured message level.
 *
 * @param level   The severity level of the message (Error, Warning, Info, Debug, Verbose).
 * @param format  printf-style format string.
 * @param ...     Additional arguments for the format string.
 */
void MessageSender::SendMessage(EMessageLevel level, const char *format, ...)
{
    // settings_message_level may be null before the messaging system is fully initialized.
    // In that case we must NOT call settings_message_level->get(), so we simply skip
    // filtering and allow the message to be sent.
    if (settings_message_level != NULL)
    {
        // If the message level is more verbose (numerically higher) than the currently
        // configured level, suppress it (do not send it).
        //
        // Example: if the configured level is Info, Debug/Verbose messages won't be sent.
        if (level > static_cast<EMessageLevel>(settings_message_level->get()))
        {
            return; // Exit early without sending anything
        }
    }

    char loc_buf[100];
    char *temp = loc_buf;

    va_list arg;
    va_start(arg, format);

    // Use a copy of arg to calculate the required buffer length,
    // so that the original arg remains intact for the actual formatting pass.
    va_list copy;
    va_copy(copy, arg);
    const int required = vsnprintf(NULL, 0, format, copy); // measure required length using copy
    va_end(copy);                                          // copy is no longer needed

    // vsnprintf returns a negative value if an encoding error occurs
    if (required < 0)
    {
        va_end(arg);
        return;
    }

    const size_t len = static_cast<size_t>(required);

    // If the message does not fit in the stack buffer, allocate on the heap
    if (len >= sizeof(loc_buf))
    {
        temp = new char[len + 1];
        if (temp == NULL)
        {
            va_end(arg);
            return;
        }
    }

    // arg is still intact here, safe to use for the final formatting pass
    vsnprintf(temp, len + 1, format, arg);
    grbl_sendf("[MSG:%s]\r\n", temp);

    va_end(arg);

    // Free heap memory if it was allocated
    if (temp != loc_buf)
    {
        delete[] temp;
    }
}

/**
 * Prints feedback messages. This serves as a centralized method to provide additional
 * user feedback for things that are not of the status/alarm message protocol. These are
 * messages such as setup warnings, switch toggling, and how to exit alarms.
 *
 * NOTE: For interfaces, messages are always placed within brackets.
 * And if silent mode is installed, the message number codes are less than zero.
 */
void MessageSender::SendFeedbackMessage(EFeedbackMessage message)
{
    const char *text = nullptr;

    switch (message)
    {
    case EFeedbackMessage::CriticalEvent:
        text = "Reset to continue";
        break;
    case EFeedbackMessage::AlarmLock:
        text = "'$H'|'$X' to unlock";
        break;
    case EFeedbackMessage::AlarmUnlock:
        text = "Caution: Unlocked";
        break;
    case EFeedbackMessage::Enabled:
        text = "Enabled";
        break;
    case EFeedbackMessage::Disabled:
        text = "Disabled";
        break;
    case EFeedbackMessage::SafetyDoorAjar:
        text = "Check door";
        break;
    case EFeedbackMessage::CheckLimits:
        text = "Check limits";
        break;
    case EFeedbackMessage::ProgramEnd:
        text = "Program End";
        break;
    case EFeedbackMessage::RestoreDefaults:
        text = "Restoring defaults";
        break;
    case EFeedbackMessage::SpindleRestore:
        text = "Restoring spindle";
        break;
    case EFeedbackMessage::SleepMode:
        text = "Sleeping";
        break;
    default:
        return; // Unknown message, do not send anything
    }

    SendMessage(EMessageLevel::Info, text);
}