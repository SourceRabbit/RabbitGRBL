#pragma once

#include <map>

#include "../Connection/MessageSender/MessageSender.h"
#include "../NVS/NVSManager.h"
#include "../Commands/Command.h"
#include "../Commands/GrblCommand.h"

// Define settings restore bitflags.
enum SettingsRestore
{
    Defaults = bit(0),
    Parameters = bit(1),
    StartupLines = bit(2),
    // BuildInfo = bit(3), // Obsolete
    Wifi = bit(4),
    All = 0xff,
};

#include "Setting.h"
#include "IntSetting.h"
#include "AxisMaskSetting.h"
#include "Coordinates.h"
#include "FloatSetting.h"
#include "StringSetting.h"
#include "EnumSetting.h"
#include "FlagSetting.h"
#include "AxisSettings.h"
#include "FakeSetting.h"
