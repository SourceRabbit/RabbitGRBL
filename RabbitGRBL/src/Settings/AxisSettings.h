#pragma once

#include "FloatSetting.h"
#include "IntSetting.h"

class AxisSettings
{
public:
    const char *name;
    FloatSetting *steps_per_mm;
    FloatSetting *max_rate;
    FloatSetting *acceleration;
    FloatSetting *max_travel;
    FloatSetting *run_current;
    FloatSetting *hold_current;
    FloatSetting *home_mpos;
    IntSetting *microsteps;
    FloatSetting *backlash;

    AxisSettings(const char *axisName);
};
