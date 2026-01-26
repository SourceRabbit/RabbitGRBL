#pragma once
#include "../Grbl.h"
#include <stdint.h>

namespace UserOutput
{
    // Base class for all user outputs (digital/analog).
    // It stores the common output identity (number) and the assigned MCU pin.
    class UserOutputBase
    {
    public:
        UserOutputBase() = default;

        /// @brief Constructs a user output with the given logical output number and assigned MCU pin.
        /// @param number Logical user output index (e.g. 0..MaxUserDigitalPin-1).
        /// @param pin MCU GPIO number used for this output. Use UNDEFINED_PIN if not configured.
        UserOutputBase(uint8_t number, uint8_t pin)
        {
            fNumber = number;
            fPinNumber = pin;
        }

        virtual ~UserOutputBase() = default;

        /// @brief Returns true when this output has a valid configured pin.
        /// @return
        bool isValid() const
        {
            return fPinNumber != UNDEFINED_PIN;
        }

        uint8_t getNumber() const { return fNumber; }
        uint8_t getPinNumber() const { return fPinNumber; }

    protected:
        // Derived classes must implement initialization.
        virtual void Initialize() = 0;

        uint8_t fNumber = UNDEFINED_PIN;
        uint8_t fPinNumber = UNDEFINED_PIN;
    };
}