/*
  Copyright (c) 2023 Nikolaos Siatras
  Twitter: nsiatras
  Website: https://www.sourcerabbit.com
*/

#include "CoolantManager.h"

Coolant CoolantManager::Mist_Coolant;
Coolant CoolantManager::Flood_Coolant;
bool CoolantManager::fInitialized = false;

/**
 * Initialize the Coolant Manager
 */
void CoolantManager::Initialize()
{
    if (!CoolantManager::fInitialized)
    {
        // Initialize Mist !
#ifdef COOLANT_MIST_PIN
#ifdef INVERT_COOLANT_MIST_PIN
        CoolantManager::Mist_Coolant.Initialize(COOLANT_MIST_PIN, true);
#else
        CoolantManager::Mist_Coolant.Initialize(COOLANT_MIST_PIN, false);
#endif
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Mist coolant on pin %s", pinName(COOLANT_MIST_PIN).c_str());
#endif

// Initialize Flood !
#ifdef COOLANT_FLOOD_PIN
#ifdef INVERT_COOLANT_FLOOD_PIN
        CoolantManager::Flood_Coolant.Initialize(COOLANT_FLOOD_PIN, true);
#else
        CoolantManager::Flood_Coolant.Initialize(COOLANT_FLOOD_PIN, false);
#endif
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Flood coolant on pin %s", pinName(COOLANT_FLOOD_PIN).c_str());
#endif
    }
    else
    {
        // If the CoolantManager has already be initialized then just turn all coolants off.
        // This case might happened after the user send a reset command to the controller.
        TurnAllCoolantsOff();
    }
    CoolantManager::fInitialized = true;
}

/**
 * Turn all coolants offs
 */
void CoolantManager::TurnAllCoolantsOff()
{
    CoolantManager::Mist_Coolant.TurnOff();
    CoolantManager::Flood_Coolant.TurnOff();
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
