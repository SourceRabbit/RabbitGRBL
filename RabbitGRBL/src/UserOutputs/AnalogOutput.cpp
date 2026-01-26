#include "../Grbl.h"
#include "../System.h"
#include "AnalogOutput.h"

namespace UserOutput
{
    AnalogOutput::AnalogOutput() {}

    AnalogOutput::AnalogOutput(uint8_t number, uint8_t pin, float pwm_frequency)
        : UserOutputBase(number, pin)
    {
        _pwm_frequency = pwm_frequency;

        if (!isValid())
        {
            return;
        }

        // Determine the highest resolution (number of precision bits) allowed by frequency.
        const uint32_t apb_frequency = getApbFrequency();

        // Determine the highest resolution (number of precision bits) allowed by frequency.
        _resolution_bits = system_calculate_pwm_precision((uint32_t)_pwm_frequency);

        // Initialize
        _pwm_channel = sys_get_next_PWM_chan_num();
        if (_pwm_channel == -1)
        {
            grbl_msg_sendf(MsgLevel::Error, "Error: out of PWM channels");
            return;
        }

        ledcSetup(_pwm_channel, _pwm_frequency, _resolution_bits);
        ledcAttachPin(this->getPinNumber(), _pwm_channel);
        ledcWrite(_pwm_channel, 0);

        config_message();
    }

    void AnalogOutput::config_message()
    {
        grbl_msg_sendf(MsgLevel::Info, "User Analog Output:%d on Pin:%s Freq:%0.0fHz", this->getNumber(), pinName(this->getPinNumber()).c_str(), _pwm_frequency);
    }

    bool AnalogOutput::set_level(uint32_t numerator)
    {
        // Look for errors, but ignore if turning off to prevent mask turn off from generating errors.
        if (!isValid())
        {
            return false;
        }

        if (_pwm_channel == -1)
        {
            grbl_msg_sendf(MsgLevel::Info, "M67 PWM channel error");
            return false;
        }

        if (_current_value == numerator)
        {
            return true;
        }

        _current_value = numerator;
        ledcWrite(_pwm_channel, numerator);

        return true;
    }
}