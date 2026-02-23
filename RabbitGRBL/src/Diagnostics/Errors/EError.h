/*
  EError.h

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

// Grbl error codes. Valid values (0-255)
enum class EError : uint8_t
{
    Ok = 0,
    ExpectedCommandLetter = 1,
    BadNumberFormat = 2,
    InvalidStatement = 3,
    NegativeValue = 4,
    SettingDisabled = 5,
    SettingStepPulseMin = 6,
    SettingReadFail = 7,
    IdleError = 8,
    SystemGcLock = 9,
    SoftLimitError = 10,
    Overflow = 11,
    MaxStepRateExceeded = 12,
    CheckDoor = 13,
    LineLengthExceeded = 14,
    TravelExceeded = 15,
    InvalidJogCommand = 16,
    SettingDisabledLaser = 17,
    HomingNoCycles = 18,
    GcodeUnsupportedCommand = 20,
    GcodeModalGroupViolation = 21,
    GcodeUndefinedFeedRate = 22,
    GcodeCommandValueNotInteger = 23,
    GcodeAxisCommandConflict = 24,
    GcodeWordRepeated = 25,
    GcodeNoAxisWords = 26,
    GcodeInvalidLineNumber = 27,
    GcodeValueWordMissing = 28,
    GcodeUnsupportedCoordSys = 29,
    GcodeG53InvalidMotionMode = 30,
    GcodeAxisWordsExist = 31,
    GcodeNoAxisWordsInPlane = 32,
    GcodeInvalidTarget = 33,
    GcodeArcRadiusError = 34,
    GcodeNoOffsetsInPlane = 35,
    GcodeUnusedWords = 36,
    GcodeG43DynamicAxisError = 37,
    GcodeMaxValueExceeded = 38,
    PParamMaxExceeded = 39,

    NumberRange = 80,  // Setting number range problem
    InvalidValue = 81, // Setting string problem

    NvsSetFailed = 100,
    NvsGetStatsFailed = 101,
    AuthenticationFailed = 110,
    Eol = 111,

};