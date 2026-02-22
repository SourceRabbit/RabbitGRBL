/*
  EFeedbackMessage.h

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

/**
 * Define Grbl feedback message codes. Valid values (0-255).
 */
enum class EFeedbackMessage : uint8_t
{
  CriticalEvent = 1,
  AlarmLock = 2,
  AlarmUnlock = 3,
  Enabled = 4,
  Disabled = 5,
  SafetyDoorAjar = 6,
  CheckLimits = 7,
  ProgramEnd = 8,
  RestoreDefaults = 9,
  SpindleRestore = 10,
  SleepMode = 11
};