// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/command.h"
#include "app/commands/param_names.h"
#include "app/commands/params.h"
#include "app/launcher.h"
#include "base/fs.h"
#include "base/launcher.h"

namespace app
{

class LaunchCommand : public Command
{
public:
  LaunchCommand();

protected:
  bool onEnabled(Context* context) override;
  void onLoadParams(const Params& params) override;
  std::vector<CommandParamInfo> onDescribeParams() const override;
  void onExecute(Context* context) override;

private:
  enum Type
  {
    Url
  };

  Type m_type = Url;
  std::string m_path;
};

LaunchCommand::LaunchCommand()
  : Command("Launch", "Launch", CmdUIOnlyFlag)
{
}

// Only web URLs are launched (never local files/programs), so the command is
// also safe in sandboxed (MSIX) builds.
bool LaunchCommand::onEnabled(Context*)
{
  return base::launcher::is_web_url(m_path);
}

void LaunchCommand::onLoadParams(const Params& params)
{
  m_path = params.get(param::kPath);

  if (m_type == Url && !m_path.empty() && m_path[0] == '/')
  {
    m_path = WEBSITE + m_path.substr(1);
  }
}

std::vector<CommandParamInfo> LaunchCommand::onDescribeParams() const
{
  return {
      {param::kPath, "URL to open; a leading / is resolved against the site.", false, ""},
  };
}

void LaunchCommand::onExecute(Context* context)
{
  switch (m_type)
  {

  case Url:
    launcher::open_url(m_path);
    break;
  }
}

std::unique_ptr<Command> CommandFactory::createLaunchCommand()
{
  return std::make_unique<LaunchCommand>();
}

} // namespace app
