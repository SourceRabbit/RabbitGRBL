/*
    RelaySpindle.cpp

    Basic on/off relay spindle.
    Any RPM (S value) above 0 turns spindle on.

    Does NOT inherit from PWM.
*/

#include "RelaySpindle.h"

namespace Spindles
{
    void Relay::Initialize()
    {
        // Resolve pins from compile-time configuration (same approach as PWM::Initialize).
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
            return; // Cannot continue without output pin
        }

        // Read RPM and delay settings (same settings objects used by PWM spindle).
        fMinRPM = settings_spindle_rpm_min->get();
        fMaxRPM = settings_spindle_rpm_max->get();

        fSpinUpDelay = (uint32_t)(settings_spindle_delay_spinup->get() * 1000.0);
        fSpinDownDelay = (uint32_t)(settings_spindle_delay_spindown->get() * 1000.0);

        // Prepare GPIO
        pinMode(fOutputPin, OUTPUT);
        if (fEnablePin != UNDEFINED_PIN)
        {
            pinMode(fEnablePin, OUTPUT);
        }
        if (fDirectionPin != UNDEFINED_PIN)
        {
            pinMode(fDirectionPin, OUTPUT);
        }

        // Ensure everything is off at start.
        fCurrentState = SpindleState::Disable;
        fOutputOn = false;
        writeDirectionPin(true); // Default direction (CW)
        writeEnablePin(false);
        setRelayOutput(false);
    }

    uint32_t Relay::setRPM(uint32_t rpm)
    {
        // Clamp to max for safety.
        rpm = (rpm > fMaxRPM) ? fMaxRPM : rpm;

        if (fOutputPin == UNDEFINED_PIN)
        {
            return rpm;
        }

        // Apply override (percent) while RPM > 0, matching PWM behavior.
        rpm = (rpm > 0) ? (rpm * sys.spindle_speed_ovr / 100) : 0;

        sys.spindle_speed = rpm;

        // Relay is simply ON when rpm > 0.
        const bool on = (rpm > 0);

        // Enable pin follows modal state and rpm.
        writeEnablePin(gc_state.modal.spindle != SpindleState::Disable);

        setRelayOutput(on);

        return rpm;
    }

    void Relay::setState(SpindleState state, uint32_t rpm)
    {
        if (sys.abort)
        {
            return; // Block during abort.
        }

        const bool wasEnabled = (fCurrentState != SpindleState::Disable);
        const bool willBeEnabled = (state != SpindleState::Disable);
        const bool directionChange = this->isReversable() && wasEnabled && willBeEnabled && (fCurrentState != state);

        if (state == SpindleState::Disable)
        {
            sys.spindle_speed = 0;
            Stop();

            if (fCurrentState != state)
            {
                delay(fSpinDownDelay);
            }
        }
        else
        {
            if (directionChange)
            {
                // Safe reverse: stop, wait, then change direction and restart.
                sys.spindle_speed = 0;
                Stop();
                delay(fSpinDownDelay);
            }

            writeDirectionPin(state == SpindleState::Cw);
            setRPM(rpm);
            writeEnablePin(true);

            if (fCurrentState != state)
            {
                delay(fSpinUpDelay);
            }
        }

        fCurrentState = state;
        sys.report_ovr_counter = 0; // Report change immediately
    }

    SpindleState Relay::getState()
    {
        if (fOutputPin == UNDEFINED_PIN)
        {
            return SpindleState::Disable;
        }

        if (!fOutputOn)
        {
            return SpindleState::Disable;
        }

        if (fDirectionPin != UNDEFINED_PIN)
        {
            return digitalRead(fDirectionPin) ? SpindleState::Cw : SpindleState::Ccw;
        }

        return SpindleState::Cw;
    }

    void Relay::Stop()
    {
        // Ensure spindle is really off.
        writeEnablePin(false);
        setRelayOutput(false);
    }

    bool Relay::isReversable()
    {
        return (fDirectionPin != UNDEFINED_PIN);
    }

    void Relay::setRelayOutput(bool on)
    {
#ifdef INVERT_SPINDLE_PWM
        // Keep compatibility with existing invert macro (even though this is not PWM).
        on = !on;
#endif
        fOutputOn = on;
        digitalWrite(fOutputPin, on);
    }

    void Relay::writeEnablePin(bool active)
    {
        if (fEnablePin == UNDEFINED_PIN)
        {
            return;
        }

        // If spindle speed is zero, force disable.
        if (sys.spindle_speed == 0)
        {
            active = false;
        }

        if (settings_spindle_enable_invert->get())
        {
            active = !active;
        }

        digitalWrite(fEnablePin, active);
    }

    void Relay::writeDirectionPin(bool clockwise)
    {
        if (fDirectionPin == UNDEFINED_PIN)
        {
            return;
        }

        digitalWrite(fDirectionPin, clockwise);
    }

    void Relay::Dispose()
    {
        Stop();

#ifdef SPINDLE_OUTPUT_PIN
        gpio_reset_pin(SPINDLE_OUTPUT_PIN);
        pinMode(SPINDLE_OUTPUT_PIN, INPUT);
#endif
#ifdef SPINDLE_ENABLE_PIN
        gpio_reset_pin(SPINDLE_ENABLE_PIN);
        pinMode(SPINDLE_ENABLE_PIN, INPUT);
#endif
#ifdef SPINDLE_DIR_PIN
        gpio_reset_pin(SPINDLE_DIR_PIN);
        pinMode(SPINDLE_DIR_PIN, INPUT);
#endif
    }
}