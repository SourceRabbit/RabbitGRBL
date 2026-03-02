/*
	MotorsManager.h

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

#pragma once
#include "../../Grbl.h"

// Include Motors
#include "Motor.h"
#include "MotorsByType/NullMotor.h"

// Select stepper implementation based on compile-time configuration
#ifdef USE_I2S_STEPS
#include "../../I2SOut.h"
#include "MotorsByType/Stepper_I2S.h"
#elif defined(USE_SOFTWARE_STEPS)
#include "MotorsByType/Stepper_Software.h"
#else
#include "MotorsByType/Stepper_RMT.h"
#endif

class MotorsManager
{
public:
	void Initialize();

	// The return value is a bitmask of axes that can home
	uint8_t setHomingMode(uint8_t homing_mask, bool isHoming);
	void setDisable(bool disable, uint8_t mask = B11111111); // default is all axes
	bool setMotorsDirection(uint8_t dir_mask);
	void Step(uint8_t step_mask);
	void Unstep();

private:
	bool fInitialized = false;
	bool fLastMotorsState = false;
	uint8_t fLastStateMask = 0;

	Motor *fMotors[MAX_AXES][MAX_GANGED];

	uint64_t fStepPulseTimeStart = esp_timer_get_time();
};
