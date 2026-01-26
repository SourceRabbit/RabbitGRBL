#pragma once

#include <stdint.h>
#include "UserOutputBase.h"

namespace UserOutput
{
    class DigitalOutput : public UserOutputBase
    {
    public:
        DigitalOutput();
        DigitalOutput(uint8_t number, uint8_t pin);

        bool set_level(bool isOn);

    protected:
        void Initialize() override;
    };
}