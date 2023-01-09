#pragma once

#include <Arduino.h>
#include <cstdint>

class Coolant
{
public:
    Coolant(uint8_t pin, bool invertPinOutput);

    virtual void TurnOn();
    virtual void TurnOnWithDelay(uint16_t delayMilliseconds);
    virtual void TurnOff();

    virtual bool getState();

protected:
    uint8_t fPinNumber;
    bool fInvertPinOutput;
};
