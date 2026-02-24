#pragma once

#include "Motor.h"

namespace Motors
{
    class Stepper_RMT : public Motor
    {
    public:
        Stepper_RMT(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

        // Overrides for inherited methods
        void Initialize() override;
        // No special action, but return true to say homing is possible
        bool setHomingMode(bool isHoming) override { return true; }
        void set_disable(bool) override;
        void set_direction(bool) override;
        void step() override;
        void unstep() override;

    protected:
        rmt_channel_t _rmt_chan_num;

        // FIX #2: Per-instance RMT config and items (previously static — shared across all axes).
        // Each stepper axis must have its own independent RMT configuration.
        rmt_item32_t _rmtItem[2];
        rmt_config_t _rmtConfig;

        bool _invert_step_pin;
        bool _invert_dir_pin;
        uint8_t _step_pin;
        uint8_t _dir_pin;
        uint8_t _disable_pin;

    private:
        // Returns the next available RMT channel, or RMT_CHANNEL_MAX if none available
        static rmt_channel_t get_next_RMT_chan_num();
    };
}
