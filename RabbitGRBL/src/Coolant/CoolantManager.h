/*
  CoolantManager.h

  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
  Website: https://www.sourcerabbit.com

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
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
