#pragma once

#include "../Grbl.h"
#include <cstdint>
#include "Coolant.h"

class CoolantManager
{
public:

    static void Initialize();
    static void TurnAllCoolantsOff();

    static Coolant Mist_Coolant;
    static Coolant Flood_Coolant;
    static void setCoolantState(CoolantState state);

private:
    static Coolant fCoolants[];
};
