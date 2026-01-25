#pragma once

/*
    RelaySpindle.h

    Basic on/off relay spindle.
    Any RPM (S value) above 0 turns spindle on.

    Does NOT inherit from PWM.
*/

#include "Spindle.h"

namespace Spindles
{
    class Relay : public Spindle
    {
    public:
        Relay() = default;

        Relay(const Relay &) = delete;
        Relay(Relay &&) = delete;
        Relay &operator=(const Relay &) = delete;
        Relay &operator=(Relay &&) = delete;

        void Initialize() override;
        uint32_t setRPM(uint32_t rpm) override;
        void setState(SpindleState state, uint32_t rpm) override;
        SpindleState getState() override;
        void Stop() override;
        bool isReversable() override;

        virtual ~Relay() {}

    protected:
        // Pins
        uint8_t fOutputPin = UNDEFINED_PIN;    // Main relay output pin
        uint8_t fEnablePin = UNDEFINED_PIN;    // Optional enable pin
        uint8_t fDirectionPin = UNDEFINED_PIN; // Optional direction pin

        // RPM range
        uint32_t fMinRPM = 0;
        uint32_t fMaxRPM = 0;

        // Delays (milliseconds)
        uint32_t fSpinUpDelay = 0;
        uint32_t fSpinDownDelay = 0;

        // Current state tracking
        SpindleState fCurrentState = SpindleState::Disable;
        bool fOutputOn = false;

        void setRelayOutput(bool on);
        void writeEnablePin(bool active);
        void writeDirectionPin(bool clockwise);
        void Dispose() override;
    };
}