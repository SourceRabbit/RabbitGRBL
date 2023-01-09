#pragma once

#include <Arduino.h>
#include <cstdint>

class Coolant
{
public:
    Coolant(uint8_t pin, bool invertPinOutput);

    virtual void TurnOn();
    virtual void TurnOff();

    virtual bool getState();

protected:
    uint8_t fPinNumber;
    bool fInvertPinOutput;
};
