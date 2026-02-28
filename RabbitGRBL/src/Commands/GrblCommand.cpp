/*
  GrblCommand.cpp

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

#include "GrblCommand.h"
#include "GRBLCommandsManager.h"

GrblCommand::GrblCommand(const char *name, const char *description, EError (*action)(const char *), bool (*cmdChecker)())
    : Command(ERabbitGRBLItemType::COMMAND, name, description, cmdChecker),
      _action(action)
{
  // Register this GrblCommand into the GRBLCommandsManager list
  GRBLCommandsManager::getGRBLCommandsList().push_back(this);
}

EError GrblCommand::action(char *value)
{
  if (_cmdChecker && _cmdChecker())
  {
    return EError::IdleError;
  }
  return _action((const char *)value);
}