// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/commands.h"

#include "app/commands/command.h"
#include "app/console.h"
#include "base/string.h"
#include "ui/ui.h"

#include <cstring>
#include <exception>

namespace app
{

#undef FOR_EACH_COMMAND
#define FOR_EACH_COMMAND(Name) const char* CommandId::Name = #Name;
#include "app/commands/commands_list.h"
#undef FOR_EACH_COMMAND

CommandsModule* CommandsModule::m_instance = nullptr;

CommandsModule::CommandsModule()
{
  ASSERT(m_instance == nullptr);
  m_instance = this;

#undef FOR_EACH_COMMAND
#define FOR_EACH_COMMAND(Name)                                                 \
  m_commands.push_back(CommandFactory::create##Name##Command());

#include "app/commands/commands_list.h"
#undef FOR_EACH_COMMAND
}

CommandsModule::~CommandsModule()
{
  ASSERT(m_instance == this);

  m_commands.clear();
  m_instance = nullptr;
}

CommandsModule* CommandsModule::instance()
{
  ASSERT(m_instance != nullptr);
  return m_instance;
}

Command* CommandsModule::getCommandByName(const char* name)
{
  if (!name)
    return nullptr;

  std::string lname = base::string_to_lower(name);
  for (const auto& cmd : m_commands)
  {
    if (base::utf8_icmp(cmd->id(), lname) == 0)
      return cmd.get();
  }

  return nullptr;
}

} // namespace app
