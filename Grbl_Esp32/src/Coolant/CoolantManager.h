/*
  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Website: https://www.sourcerabbit.com
*/

#pragma once

#include "../Grbl.h"
#include <cstdint>
#include "Coolant.h"

class CoolantManager
{
public:
  static Coolant Mist_Coolant;
  static Coolant Flood_Coolant;

  static void Initialize();
  static void TurnAllCoolantsOff();
  static void setCoolantState(CoolantState state);

  static bool AreAllCoolantsOff();

private:
  static bool fInitialized;
  static Coolant *fCoolants[];
};
