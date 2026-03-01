/*
  ECoordinatesIndex.h

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

#pragma once

// Various places in the code access saved coordinate system data
// by a small integer index according to the values below.
enum ECoordinatesIndex : uint8_t
{
    Begin = 0,
    G54 = Begin,
    G55,
    G56,
    G57,
    G58,
    G59,
    // To support 9 work coordinate systems it would be necessary to define
    // the following 3 and modify GCode.cpp to support G59.1, G59.2, G59.3
    // G59_1,
    // G59_2,
    // G59_3,
    NWCSystems,
    G28 = NWCSystems,
    G30,
    // G92_2,
    // G92_3,
    End,
};

// Allow iteration over ECoordinatesIndex values
ECoordinatesIndex &operator++(ECoordinatesIndex &i);
