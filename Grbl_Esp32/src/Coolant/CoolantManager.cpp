#include "CoolantManager.h"
#include "Coolant.h"

#ifdef COOLANT_MIST_PIN
#ifdef INVERT_COOLANT_MIST_PIN
Coolant CoolantManager::Mist_Coolant(COOLANT_MIST_PIN, true);
#else
Coolant CoolantManager::Mist_Coolant(COOLANT_MIST_PIN, false);
#endif
#else
Coolant CoolantManager::Mist_Coolant(0, false);
#endif

#ifdef COOLANT_FLOOD_PIN
#ifdef INVERT_COOLANT_FLOOD_PIN
Coolant CoolantManager::Flood_Coolant(COOLANT_FLOOD_PIN, true);
#else
Coolant CoolantManager::Flood_Coolant(COOLANT_FLOOD_PIN, false);
#endif
#else
Coolant CoolantManager::Flood_Coolant(0, false);
#endif

CoolantManager::CoolantManager()
{
}

/**
 * Initialize the Coolant Manager
 */
void CoolantManager::Initialize()
{
    CoolantManager::TurnAllCoolantsOff();
}

/**
 * Turn all coolants offs
 */
void CoolantManager::TurnAllCoolantsOff()
{
    CoolantManager::Mist_Coolant.TurnOff();
    CoolantManager::Flood_Coolant.TurnOff();
}
