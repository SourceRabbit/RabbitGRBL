#include "../Grbl.h"
#include "Coolant.h"

Coolant::Coolant()
{
}

void Coolant::Initialize(uint8_t pin, bool invertPinOutput)
{
    fPinNumber = pin;
    fInvertPinOutput = invertPinOutput;

    if (fPinNumber > 0)
    {
        pinMode(fPinNumber, OUTPUT);
    }

    // Turn off after initialization
    this->TurnOff();
}

/**
 * Turn the coolant On.
 */
void Coolant::TurnOn()
{
    if (fPinNumber > 0)
    {
        digitalWrite(fPinNumber, (fInvertPinOutput) ? 0 : 1);
        fIsOn = true;
    }
}

/**
 * Turn the coolant On and wait.
 */
void Coolant::TurnOnWithDelay(uint16_t delayMilliseconds)
{
    if (fPinNumber > 0)
    {
        this->TurnOn();
        delay(delayMilliseconds);
    }
}

/**
 * Turn the coolant Off.
 */
void Coolant::TurnOff()
{
    if (fPinNumber > 0)
    {
        digitalWrite(fPinNumber, (fInvertPinOutput) ? 1 : 0);
    }
    fIsOn = false;
}

/**
 * Toggle will change the coolant's status. If the coolant is On then
 * it will turn Off, otherwise it will turn On.
 */
void Coolant::Toggle()
{
    if (this->getState())
    {
        this->TurnOff();
    }
    else
    {
        this->TurnOn();
    }

    sys.report_ovr_counter = 0; // Set to report change immediately
}

/**
 * Returns true if the Coolant is on
 */
bool Coolant::getState()
{
    return fIsOn;
}