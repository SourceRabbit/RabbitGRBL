#pragma once

#include <stdint.h>

enum class ERabbitGRBLItemType : uint8_t
{
    GRBL = 1,     // Classic GRBL settings like $100
    EXTENDED = 2, // Settings added by early versions of Grbl_Esp32
    GRBLCMD = 3   // Non-persistent GRBL commands like $H
};
