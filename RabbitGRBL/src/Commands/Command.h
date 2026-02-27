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

// Command::List is a linked list of all commands,
// so common code can enumerate them.
class Command : public Word
{
protected:
    Command *link; // linked list of command objects
    bool (*_cmdChecker)();

public:
    static Command *List;
    Command *next() { return link; }

    ~Command() {}
    Command(const char *description, ERabbitGRBLItemType type, const char *grblName, const char *fullName, bool (*cmdChecker)());

    virtual EError action(char *value) = 0;
};
