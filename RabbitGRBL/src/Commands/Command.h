/*
  Command.h

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

#include "../Settings/Word.h"
#include "../Diagnostics/Errors/EError.h"

// Command is the base class for all GRBL command objects.
class Command : public Word
{
protected:
    bool (*_cmdChecker)();

public:
    ~Command() {}
    Command(ERabbitGRBLItemType type, const char *name, const char *description, bool (*cmdChecker)());

    virtual EError action(char *value) = 0;
};