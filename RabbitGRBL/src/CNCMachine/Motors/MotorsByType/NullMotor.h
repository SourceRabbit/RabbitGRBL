#pragma once

#include "../../../Grbl.h"
#include "../Motor.h"

class Nullmotor : public Motor
{
public:
    Nullmotor(uint8_t axis_index);
    bool setHomingMode(bool isHoming) { return false; }
};
