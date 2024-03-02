#include "Grbl.h"

String pinName(uint8_t pin)
{
    if (pin == UNDEFINED_PIN)
    {
        return "None";
    }

    return String("GPIO(") + pin + ")";
}

// Even if USE_I2S_OUT is not defined, it is necessary to
// override the following functions, instead of allowing
// the weak aliases in the library to apply, because of
// the UNDEFINED_PIN check.  That UNDEFINED_PIN behavior
// cleans up other code by eliminating ifdefs and checks.
void IRAM_ATTR digitalWrite(uint8_t pin, uint8_t val)
{
    if (pin == UNDEFINED_PIN)
    {
        return;
    }

    __digitalWrite(pin, val);
}

void IRAM_ATTR pinMode(uint8_t pin, uint8_t mode)
{
    if (pin == UNDEFINED_PIN)
    {
        return;
    }

    __pinMode(pin, mode);

    // I2S out pins cannot be configured, hence there
    // is nothing to do here for them.
}

int IRAM_ATTR digitalRead(uint8_t pin)
{
    if (pin == UNDEFINED_PIN)
    {
        return 0;
    }

    return __digitalRead(pin);
}
