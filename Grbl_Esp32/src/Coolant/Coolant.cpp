#include "Coolant.h"

Coolant::Coolant(uint8_t pin, bool invertPinOutput)
{
    fPinNumber = pin;
    fInvertPinOutput = pin;
    pinMode(fPinNumber, OUTPUT);
}

void Coolant::TurnOn()
{
    digitalWrite(fPinNumber, (fInvertPinOutput) ? false : true);
}

void Coolant::TurnOnWithDelay(uint16_t delayMilliseconds)
{
    this->TurnOn();
    delay(delayMilliseconds);
}

void Coolant::TurnOff()
{
    digitalWrite(fPinNumber, (fInvertPinOutput) ? true : false);
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