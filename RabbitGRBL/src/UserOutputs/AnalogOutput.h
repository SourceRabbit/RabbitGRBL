#pragma once

#include <stdint.h>
#include "UserOutputBase.h"

namespace UserOutput
{
    class AnalogOutput : public UserOutputBase
    {
    public:
        AnalogOutput();
        AnalogOutput(uint8_t number, uint8_t pin, float pwm_frequency);

        bool set_level(uint32_t numerator);
        uint32_t denominator() const { return 1UL << _resolution_bits; }

    protected:
        void Initialize() override;
        void config_message();

        int8_t _pwm_channel = -1; // -1 means invalid or not setup
        float _pwm_frequency = 0.0f;
        uint8_t _resolution_bits = 0;
        uint32_t _current_value = 0;
    };
}