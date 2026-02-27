#pragma once

#include <stdint.h>

enum class ERabbitGRBLItemType : uint8_t
{
    GRBL = 1,     // Classic GRBL settings like $100
    EXTENDED = 2, // Extended Settings
    GRBLCMD = 3   // Non-persistent GRBL commands like $H
};
