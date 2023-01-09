#pragma once

#include <cstdint>
#include "Coolant.h"

class CoolantManager
{
public:
    CoolantManager();
    static void Initialize();
    static void TurnAllCoolantsOff();

    static Coolant Mist_Coolant;
    static Coolant Flood_Coolant;
};

