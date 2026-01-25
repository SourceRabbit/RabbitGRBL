/*
  Spindle.cpp

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
  Github: https://github.com/nsiatras
  Website: https://www.sourcerabbit.com

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Spindle.h"

#include "NullSpindle.h"
#include "PWMSpindle.h"
#include "RelaySpindle.h"
// #include "Laser.h"

namespace Spindles
{
    // An instance of each type of spindle is created here.
    // This allows the spindle to be dynamicly switched
    Null null;
    PWM pwm;
    Relay relay;
    // Laser laser;

    void Spindle::Select()
    {
        switch (static_cast<ESpindleType>(settings_spindle_type->get()))
        {
        case ESpindleType::PWM:
            fSpindle = &pwm;
            break;
        case ESpindleType::RELAY:
            fSpindle = &relay;
            break;

        case ESpindleType::LASER:
        case ESpindleType::NONE:
        default:
            fSpindle = &null;
            break;
        }

        fSpindle->Initialize();
    }

    void Spindle::Initialize()
    {
        // Default pins to "undefined" unless provided by machine configuration.
        fOutputPin = UNDEFINED_PIN;
#ifdef SPINDLE_OUTPUT_PIN
        fOutputPin = SPINDLE_OUTPUT_PIN;
#endif

        fEnablePin = UNDEFINED_PIN;
#ifdef SPINDLE_ENABLE_PIN
        fEnablePin = SPINDLE_ENABLE_PIN;
#endif

        fDirectionPin = UNDEFINED_PIN;
#ifdef SPINDLE_DIR_PIN
        fDirectionPin = SPINDLE_DIR_PIN;
#endif

        if (fOutputPin == UNDEFINED_PIN)
        {
            grbl_msg_sendf(MsgLevel::Info, "Warning: SPINDLE_OUTPUT_PIN not defined");
            return; // We cannot continue without the output pin
        }

        // Setup delays (milliseconds)
        fSpinUpDelay = settings_spindle_delay_spinup->get() * 1000.0;
        fSpinDownDelay = settings_spindle_delay_spindown->get() * 1000.0;
    }

    void Spindle::Synch(SpindleState state, uint32_t rpm)
    {
        if (sys.state == State::CheckMode)
        {
            return;
        }
        protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
        setState(state, rpm);
    }

    /// @brief Return the configured spin-up delay (milliseconds) for this spindle instance.
    /// @return Spin-up delay in milliseconds.
    uint32_t Spindle::getSpinUpDelay()
    {
        return fSpinUpDelay;
    }

    /// @brief Return the configured spin-down delay (milliseconds) for this spindle instance.
    /// @return Spin-down delay in milliseconds.
    uint32_t Spindle::getSpinDownDelay()
    {
        return fSpinDownDelay;
    }

    /// @brief Check whether spin-up/spin-down delays are enabled for this spindle instance.
    /// @return True if either the spin-up or spin-down delay is greater than 0 ms; otherwise false.
    bool Spindle::isUsingDelays()
    {
        // Delays are considered "in use" when at least one of the delay values is non-zero.
        return (fSpinUpDelay > 0) || (fSpinDownDelay > 0);
    }

    bool Spindle::inLaserMode()
    {
        return false; 
    }

    /// @brief Check whether the spindle supports direction reversing (CW/CCW).
    /// @return True if a direction pin is defined (hardware supports reversing); otherwise false.
    bool Spindle::isReversable()
    {
        // Reversing is possible only when a valid direction GPIO pin is configured.
        return fDirectionPin != UNDEFINED_PIN;
    }

    /// @brief Disposes the spindle
    void Spindle::Dispose()
    {
        Stop();
    }
}

// Declares the machine's spindle
Spindles::Spindle *fSpindle;