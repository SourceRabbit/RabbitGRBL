/*
  Coordinates.h

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

class Coordinates
{
private:
    float fCurrentValue[MAX_N_AXIS];
    const char *fName;

public:
    Coordinates(const char *name) : fName(name) {}

    bool LoadFromNVS();

    const char *getName() { return fName; }

    void setDefault()
    {
        float zeros[MAX_N_AXIS] = {
            0.0,
        };
        set(zeros);
    };

    // Copy the value to an array
    void get(float *value) { memcpy(value, fCurrentValue, sizeof(fCurrentValue)); }

    // Return a pointer to the array
    const float *get() { return fCurrentValue; }

    void set(float *value);
};
