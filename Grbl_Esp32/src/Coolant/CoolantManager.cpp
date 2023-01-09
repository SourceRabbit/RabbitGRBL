#include "CoolantManager.h"

Coolant CoolantManager::Mist_Coolant;
Coolant CoolantManager::Flood_Coolant;

/**
 * Initialize the Coolant Manager
 */
void CoolantManager::Initialize()
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

/**
 * Turn all coolants offs
 */
void CoolantManager::TurnAllCoolantsOff()
{
    CoolantManager::Mist_Coolant.TurnOff();
    CoolantManager::Flood_Coolant.TurnOff();
    sys.report_ovr_counter = 0; // Set to report change immediately
}

void CoolantManager::setCoolantState(CoolantState state)
{
    if (state.Mist == 1)
    {
        CoolantManager::Mist_Coolant.TurnOn();
    }
    else
    {
        CoolantManager::Mist_Coolant.TurnOff();
    }

    if (state.Flood == 1)
    {
        CoolantManager::Flood_Coolant.TurnOn();
    }
    else
    {
        CoolantManager::Flood_Coolant.TurnOff();
    }

    sys.report_ovr_counter = 0; // Set to report change immediately
}
