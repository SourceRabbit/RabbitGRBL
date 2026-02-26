/*
CoolantManager.cpp

Copyright (c) 2023 Nikolaos Siatras
Twitter: nsiatras
Github: https://github.com/nsiatras
Website: https://www.sourcerabbit.com

Rabbit GRBL is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Rabbit GRBL is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CoolantManager.h"
#include "../../Connection/MessageSender/MessageSender.h"

/**
 * Initialize the Coolant Manager
 */
void CoolantManager::Initialize()
{
    if (fInitialized)
    {
        // If the CoolantManager has already been initialized then just turn all coolants off.
        // This case might happened after the user send a reset command to the controller.
        TurnAllCoolantsOff();
        return;
    }

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize Mist (M7)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef COOLANT_MIST_PIN
    Mist_Coolant.Initialize(COOLANT_MIST_PIN, INVERT_COOLANT_MIST_PIN, settings_coolant_mist_start_delay);
//  MessageSender::SendMessage(EMessageLevel::Info, "Mist coolant on pin %s", pinName(COOLANT_MIST_PIN).c_str());
#else
    Mist_Coolant.Initialize(0, true);
#endif
    fCoolants[0] = &Mist_Coolant;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize Flood (M8)
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef COOLANT_FLOOD_PIN
    Flood_Coolant.Initialize(COOLANT_FLOOD_PIN, INVERT_COOLANT_FLOOD_PIN, settings_coolant_flood_start_delay);
//  MessageSender::SendMessage(EMessageLevel::Info, "Flood coolant on pin %s", pinName(COOLANT_FLOOD_PIN).c_str());
#else
    Flood_Coolant.Initialize(0, true);
#endif
    fCoolants[1] = &Flood_Coolant;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Finally mark the CoolantManager as Initialized !
    fInitialized = true;

    MessageSender::SendMessage(EMessageLevel::Info, "Coolant Manager Initialized");
}

/**
 * Turns all coolants off
 */
void CoolantManager::TurnAllCoolantsOff()
{
    for (int i = 0; i < COOLANTS_COUNT; i++)
    {
        fCoolants[i]->TurnOff();
    }

    sys.report_ovr_counter = 0; // Set to report change immediately
}

/**
 * Sets all coolant states according to the given CoolantState.
 * Uses the fCoolants array to iterate over all coolants and set their state.
 */
void CoolantManager::setCoolantState(CoolantState state)
{
    // Build a matching state array (index 0 = Mist, index 1 = Flood)
    bool states[COOLANTS_COUNT] = {state.Mist == 1, state.Flood == 1};

    for (int i = 0; i < COOLANTS_COUNT; i++)
    {
        fCoolants[i]->setState(states[i]);
    }

    sys.report_ovr_counter = 0; // Set to report change immediately
}

/**
 * Returns true if all Coolants are Off
 */
bool CoolantManager::AreAllCoolantsOff()
{
    for (int i = 0; i < COOLANTS_COUNT; i++)
    {
        if (fCoolants[i]->isOn())
        {
            return false;
        }
    }
    return true;
}
