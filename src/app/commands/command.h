// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/commands/command_factory.h"
#include <string>

namespace app
{
class Context;
class Params;

enum [[clang::flag_enum]] CommandFlags
{
  CmdUIOnlyFlag = 0x00000001,
  CmdRecordableFlag = 0x00000002,
};

class Command
{
public:
  Command(const char* id, const char* friendlyName, CommandFlags flags);
  virtual ~Command() = default;

  // Commands are singletons owned by CommandsModule; nothing ever needs to
  // copy one, and deleting these turns "does anything still copy commands?"
  // into a compile error (see issue #224).
  Command(const Command&) = delete;
  Command& operator=(const Command&) = delete;

  const std::string& id() const { return m_id; }
  std::string friendlyName() const;

  // Was previously never stored (no m_flags member existed), making every
  // CmdUIOnlyFlag/CmdRecordableFlag call site across the codebase a silent
  // no-op (see issue #219, Phase 5). Stored now; nothing reads it yet -
  // the ~125 declared values were never validated against real command
  // behavior, so wiring up a consumer needs its own separate audit rather
  // than piggybacking on this fix.
  [[nodiscard]] CommandFlags flags() const { return m_flags; }

  void loadParams(const Params& params);
  bool isEnabled(Context* context);
  bool isChecked(Context* context);
  void execute(Context* context);

protected:
  virtual void onLoadParams(const Params& params);
  virtual bool onEnabled(Context* context);
  virtual bool onChecked(Context* context);
  virtual void onExecute(Context* context);
  virtual std::string onGetFriendlyName() const;

private:
  std::string m_id;
  std::string m_friendlyName;
  CommandFlags m_flags;
};

} // namespace app
