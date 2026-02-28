#pragma once

#include <stdint.h>

enum class ERabbitGRBLItemType : uint8_t
{
    SETTING = 1,     // Classic GRBL settings like $100
    EXTENDED_SETTING = 2, // Extended Settings
    COMMAND = 3   // Non-persistent GRBL commands like $H
};
