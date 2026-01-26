#include "../Grbl.h"
#include "DigitalOutput.h"

namespace UserOutput
{
    DigitalOutput::DigitalOutput() {}

    DigitalOutput::DigitalOutput(uint8_t number, uint8_t pin)
        : UserOutputBase(number, pin)
    {
        if (!isValid())
        {
            return;
        }

        // Initialize
        pinMode(this->getPinNumber(), OUTPUT);
        digitalWrite(this->getPinNumber(), LOW);

        grbl_msg_sendf(MsgLevel::Info, "User Digital Output:%d on Pin:%s", this->getNumber(), pinName(this->getPinNumber()).c_str());
    }

    bool DigitalOutput::set_level(bool isOn)
    {
        // Keep original behavior: if output is "not configured", reject turning it on.
        if (this->getPinNumber() == UNDEFINED_PIN && isOn)
        {
            return false;
        }

        if (!isValid())
        {
            return false;
        }

        digitalWrite(this->getPinNumber(), isOn);
        return true;
    }
}