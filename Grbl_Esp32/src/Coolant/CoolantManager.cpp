/*
  CoolantManager.cpp

  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
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

#include "CoolantManager.h"

#define COOLANTS_COUNT 2
Coolant CoolantManager::Mist_Coolant;
Coolant CoolantManager::Flood_Coolant;
Coolant *CoolantManager::fCoolants[COOLANTS_COUNT];

bool CoolantManager::fInitialized = false;

/**
 * Initialize the Coolant Manager
 */
void CoolantManager::Initialize()
{
        if (CoolantManager::fInitialized)
        {
                // If the CoolantManager has already been initialized then just turn all coolants off.
                // This case might happened after the user send a reset command to the controller.
                TurnAllCoolantsOff();
                return;
        }

        bool invertPin = false;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize Mist (M7)
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef INVERT_COOLANT_MIST_PIN
        // Check if Mist pin is inverted
        invertPin = true;
#endif
#ifdef COOLANT_MIST_PIN
        CoolantManager::Mist_Coolant.Initialize(COOLANT_MIST_PIN, invertPin);
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Mist coolant on pin %s", pinName(COOLANT_MIST_PIN).c_str());
#else
        CoolantManager::Mist_Coolant.Initialize(0, true);
#endif
        CoolantManager::fCoolants[0] = &CoolantManager::Mist_Coolant;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Initialize Flood (M8)
        /////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef INVERT_COOLANT_FLOOD_PIN
        // Check if Flood Pin is inverted
        invertPin = true;
#endif
#ifdef COOLANT_FLOOD_PIN
        CoolantManager::Flood_Coolant.Initialize(COOLANT_FLOOD_PIN, invertPin);
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Flood coolant on pin %s", pinName(COOLANT_FLOOD_PIN).c_str());
#endif
        CoolantManager::fCoolants[1] = &CoolantManager::Flood_Coolant;

        // Finally mark the  CoolantManager as Initialized !
        CoolantManager::fInitialized = true;
}

/**
 * Turn all coolants offs
 */
void CoolantManager::TurnAllCoolantsOff()
{
        for (int i = 0; i < COOLANTS_COUNT; i++)
        {
                CoolantManager::fCoolants[i]->TurnOff();
        }

        sys.report_ovr_counter = 0; // Set to report change immediately
}

/**
 * Sets all coolant states according to the given CoolantState
 */
void CoolantManager::setCoolantState(CoolantState state)
{
        CoolantManager::Mist_Coolant.setState(state.Mist == 1);
        CoolantManager::Flood_Coolant.setState(state.Flood == 1);
        sys.report_ovr_counter = 0; // Set to report change immediately
}

/**
 * Returns true if all Coolants are Off
 */
bool CoolantManager::AreAllCoolantsOff()
{
        for (int i = 0; i < COOLANTS_COUNT; i++)
        {
                if (CoolantManager::fCoolants[i]->isOn())
                {
                        return false;
                }
        }
        return true;
}
