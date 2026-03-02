/*
  Diagnostic.h

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

#include <Arduino.h>

class Diagnostic
{
private:
    uint8_t fID;
    const char *fTitle;
    const char *fDescription;

public:
    // Constructor accepting a uint8_t ID directly
    Diagnostic(uint8_t id, const char *title, const char *description)
        : fID(id), fTitle(title), fDescription(description)
    {
    }

    // Templated constructor accepting any enum class with uint8_t underlying type
    template <typename TEnum>
    Diagnostic(TEnum id, const char *title, const char *description)
        : fID(static_cast<uint8_t>(id)), fTitle(title), fDescription(description)
    {
    }

    // Returns the diagnostic ID
    uint8_t getID() const
    {
        return fID;
    }

    // Returns the diagnostic title
    const char *getTitle() const
    {
        return fTitle;
    }

    // Returns the diagnostic description
    const char *getDescription() const
    {
        return fDescription;
    }
};