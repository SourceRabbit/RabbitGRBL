#include "Coolant.h"

Coolant::Coolant(uint8_t pin, bool invertPinOutput)
{
    fPinNumber = pin;
    fInvertPinOutput = pin;

    pinMode(fPinNumber, OUTPUT);
}

void Coolant::TurnOn()
{
}

void Coolant::TurnOff()
{
}

/**
 * Returns true if the Coolant is on
*/
bool Coolant::getState()
{
    bool isCoolantOn = digitalRead(fPinNumber);
    if (fInvertPinOutput)
    {
        isCoolantOn = !isCoolantOn;
    }
    return isCoolantOn;
}