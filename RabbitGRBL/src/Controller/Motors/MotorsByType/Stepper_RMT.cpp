/*
    Stepper_RMT.cpp

    Copyright (c) 2026 Nikolaos Siatras
    Twitter: nsiatras
    Github: https://github.com/nsiatras
    Website: https://www.sourcerabbit.com

    Rabbit GRBL is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Rabbit GRBL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Rabbit GRBL.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Stepper_RMT.h"

Stepper_RMT::Stepper_RMT(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin)
    : Motor(axis_index), _step_pin(step_pin), _dir_pin(dir_pin), _disable_pin(disable_pin)
{
    // Assign a unique RMT channel to this stepper instance
    _rmt_chan_num = get_next_RMT_chan_num();
}

void Stepper_RMT::Initialize()
{
    _invert_step_pin = bitnum_istrue(step_invert_mask->get(), fAxisIndex);
    _invert_dir_pin = bitnum_istrue(dir_invert_mask->get(), fAxisIndex);
    pinMode(_dir_pin, OUTPUT);

    if (_rmt_chan_num == RMT_CHANNEL_MAX)
    {
        MessageSender::SendMessage(EMessageLevel::Error, "Error: invalid RMT channel for axis %d", fAxisIndex);
        return;
    }

    _rmtConfig.rmt_mode = RMT_MODE_TX;
    _rmtConfig.clk_div = 20;
    _rmtConfig.mem_block_num = 1;
    _rmtConfig.tx_config.loop_en = false;
    _rmtConfig.tx_config.carrier_en = false;
    _rmtConfig.tx_config.carrier_freq_hz = 0;
    _rmtConfig.tx_config.carrier_duty_percent = 50;
    _rmtConfig.tx_config.carrier_level = RMT_CARRIER_LEVEL_LOW;
    _rmtConfig.tx_config.idle_output_en = true;

    auto stepPulseDelay = direction_delay_microseconds->get();
    _rmtItem[0].duration0 = stepPulseDelay < 1 ? 1 : stepPulseDelay * 4;
    _rmtItem[0].duration1 = 4 * pulse_microseconds->get();
    _rmtItem[1].duration0 = 0;
    _rmtItem[1].duration1 = 0;

    _rmtConfig.channel = _rmt_chan_num;
    _rmtConfig.tx_config.idle_level = _invert_step_pin ? RMT_IDLE_LEVEL_HIGH : RMT_IDLE_LEVEL_LOW;
    _rmtConfig.gpio_num = gpio_num_t(_step_pin);

    _rmtItem[0].level0 = _rmtConfig.tx_config.idle_level;
    _rmtItem[0].level1 = !_rmtConfig.tx_config.idle_level;

    // Configure and install RMT driver
    rmt_config(&_rmtConfig);
    rmt_set_source_clk(_rmt_chan_num, RMT_BASECLK_APB);
    rmt_driver_install(_rmt_chan_num, 0, 0);

    // Pre-load the RMT items ONCE here at initialization
    // Step() will only trigger transmission - no item loading needed at ISR time
    rmt_fill_tx_items(_rmt_chan_num, _rmtItem, 2, 0);

    pinMode(_disable_pin, OUTPUT);

    /*MessageSender::SendMessage(EMessageLevel::Info,
                               "%s RMT Stepper Step:%s Dir:%s Disable:%s %s",
                               reportAxisNameMsg(fAxisIndex, fDualAxisIndex),
                               pinName(_step_pin).c_str(),
                               pinName(_dir_pin).c_str(),
                               pinName(_disable_pin).c_str(),
                               reportAxisLimitsMsg(fAxisIndex));*/
}

// FIX #1: get_next_RMT_chan_num() - Fixed off-by-one bug.
// Previously, the function incremented BEFORE returning, skipping channel 0
// and potentially returning RMT_CHANNEL_MAX (invalid) as a usable channel.
// Now it returns the current channel first, then increments (post-increment).
rmt_channel_t Stepper_RMT::get_next_RMT_chan_num()
{
    static uint8_t next_RMT_chan_num = uint8_t(RMT_CHANNEL_0); // Start at channel 0

    if (next_RMT_chan_num >= uint8_t(RMT_CHANNEL_MAX))
    {
        // No more RMT channels available
        MessageSender::SendMessage(EMessageLevel::Error, "Error: out of RMT channels");
        return RMT_CHANNEL_MAX; // Return explicit error value without incrementing
    }

    return rmt_channel_t(next_RMT_chan_num++); // Return current channel, then increment
}

void Stepper_RMT::Step()
{
    // Items are pre-loaded in Initialize() - just trigger transmission here.
    // rmt_tx_start() is ISR-safe: it only writes to hardware registers.
    // Do NOT use rmt_write_items() here - it uses semaphores and will crash in ISR context!
    rmt_tx_start(_rmt_chan_num, true);
}

void Stepper_RMT::Unstep()
{
    // With RMT, the pulse end is handled automatically by hardware — no action needed here.
}

void Stepper_RMT::setDirection(bool dir)
{
    digitalWrite(_dir_pin, dir ^ _invert_dir_pin);
}

void Stepper_RMT::setDisable(bool disable)
{
    digitalWrite(_disable_pin, disable);
}
