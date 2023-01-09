#pragma once

#include <Arduino.h>
#include <cstdint>

class Coolant
{
public:
    Coolant();

    virtual void Initialize(uint8_t pin, bool invertPinOutput);

    virtual void TurnOn();
    virtual void TurnOnWithDelay(uint16_t delayMilliseconds);
    virtual void TurnOff();
    void Toggle();
    virtual bool getState();

protected:
    bool fIsOn = false;
    uint8_t fPinNumber = 0;
    bool fInvertPinOutput = false;
};
