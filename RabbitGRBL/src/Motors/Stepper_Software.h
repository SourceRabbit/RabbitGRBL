#pragma once

#include "Motor.h"

namespace Motors
{
    class Stepper_Software : public Motor
    {
    public:
        Stepper_Software(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

        // Overrides for inherited methods
        void Initialize() override;
        // No special action, but return true to say homing is possible
        bool setHomingMode(bool isHoming) override { return true; }
        void setDisable(bool) override;
        void setDirection(bool) override;
        void Step() override;
        void Unstep() override;

    protected:
        bool _invert_step_pin;
        bool _invert_dir_pin;
        uint8_t _step_pin;
        uint8_t _dir_pin;
        uint8_t _disable_pin;
    };
}
