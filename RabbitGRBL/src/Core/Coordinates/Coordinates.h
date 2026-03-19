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
protected:
    float fCurrentValue[MAX_N_AXIS];
    const char *fName;

public:
    Coordinates(const char *name) : fName(name)
    {
    }

    virtual ~Coordinates()
    {
    }

    const char *getName()
    {
        return fName;
    }

    // Returns true if this coordinate system is persistent (saved to NVS).
    // Base class returns false; PersistentCoordinates overrides to return true.
    virtual bool isPersistent()
    {
        return false;
    }

    void setDefault()
    {
        float zeros[MAX_N_AXIS] = {
            0.0,
        };
        set(zeros);
    }

    // Copy the value to an array
    void get(float *value)
    {
        memcpy(value, fCurrentValue, sizeof(fCurrentValue));
    }

    // Return a pointer to the array
    const float *get()
    {
        return fCurrentValue;
    }

    virtual void set(const float *value);

    // Returns a formatted string in the form "[<name>:<x>,<y>,<z>,...]\r\n"
    String toString();
};
