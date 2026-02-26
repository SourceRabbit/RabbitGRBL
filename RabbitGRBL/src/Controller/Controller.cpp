/*
  Controller.cpp

  Copyright (c) 2026 Nikolaos Siatras
  Twitter: nsiatras
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

#include "../Grbl.h"
#include "Controller.h"
#include "Coolant/CoolantManager.h"
#include "Probe/Probe.h"
#include "Backlash/BacklashManager.h"
#include "Motors/MotorsManager.h"
#include "Spindles/Spindle.h"
#include "UserOutputs/UserOutputsManager.h"

static BacklashManager fBacklashManagerInstance;
static MotorsManager fMotorsManagerInstance;
static Spindle *fSpindleInstance = nullptr;
static CoolantManager fCoolantManagerInstance;
static Probe fProbeInstance;
static UserOutputsManager fUserOutputsManagerInstance;

/**
 * Initializes the controller
 */
void Controller::Initialize()
{

    // Initialize BacklashManager and Motors
    // Note: BacklashManager has to be initialized before motors!
    fBacklashManagerInstance.Initialize();
    fMotorsManagerInstance.Initialize();

    // Select the Spindle !
    selectSpindle();

    fCoolantManagerInstance.Initialize();
    fProbeInstance.Initialize();
    fUserOutputsManagerInstance.Initialize();
}

/**
 * Returns the BacklashManager instance
 */
BacklashManager &Controller::getBacklashManager()
{
    return fBacklashManagerInstance;
}

/**
 * Returns the MotorsManager instance
 */
MotorsManager &Controller::getMotorsManager()
{
    return fMotorsManagerInstance;
}

/**
 * Returns the CoolantManager instance
 */
CoolantManager &Controller::getCoolantManager()
{
    return fCoolantManagerInstance;
}

/**
 * Returns the active Spindle instance
 */
Spindle *Controller::getSpindle()
{
    return fSpindleInstance;
}

/**
 * Selects and initializes the spindle based on current settings
 */
void Controller::selectSpindle()
{
    Spindle::Select(fSpindleInstance);
}

/**
 * Returns the Probe instance
 */
Probe &Controller::getProbe()
{
    return fProbeInstance;
}

/**
 * Returns the UserOutputsManager instance
 */
UserOutputsManager &Controller::getUserOutputsManager()
{
    return fUserOutputsManagerInstance;
}
