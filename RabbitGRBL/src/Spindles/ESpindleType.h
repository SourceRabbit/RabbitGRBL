#pragma once
#include <cstdint>

enum class ESpindleType : int8_t
{
    NONE = 0,
    PWM = 1,
    RELAY = 2,
    LASER = 3,
    BESC = 4,
};