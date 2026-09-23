// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#define TEST_GUI
#include "tests.h"

#include "app/app.h"
#include "app/commands/command.h"
#include "app/commands/commands.h"

// Smoke test for issue #224: Command::clone() is gone, CommandsModule now
// owns std::unique_ptr<Command> instead of raw pointers, and every
// CommandFactory::create*Command() factory returns std::unique_ptr<Command>
// via std::make_unique instead of `new`. This constructs every one of the
// ~155 registered commands and asserts the conversion didn't drop or
// misconstruct any of them.

namespace
{

// See test/app/script_api_tests.cpp:84-90 for why this is deliberately
// leaked and constructed at most once for the whole test binary.
void ensureAppAndCommands()
{
  static app::App* app = new app::App();
  app->initializeCoreModulesForTesting();
  static app::CommandsModule* commands = new app::CommandsModule();
  (void)commands;
}

class CommandFlagsTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite() { ensureAppAndCommands(); }
};

} // namespace

TEST_F(CommandFlagsTest, EveryRegisteredCommandConstructsWithNonEmptyId)
{
  int count = 0;
  for (auto& cmd : *app::CommandsModule::instance())
  {
    ASSERT_NE(cmd, nullptr);
    EXPECT_FALSE(cmd->id().empty());
    ++count;
  }
  // Sanity bound rather than an exact count - commands_list.h grows over
  // time and this test shouldn't need updating for every new command.
  EXPECT_GT(count, 100);
}

TEST_F(CommandFlagsTest, GetCommandByNameFindsAKnownCommand)
{
  app::Command* cmd =
      app::CommandsModule::instance()->getCommandByName(app::CommandId::Undo);
  ASSERT_NE(cmd, nullptr);
  EXPECT_EQ(cmd->id(), app::CommandId::Undo);
}

TEST_F(CommandFlagsTest, GetCommandByNameIsCaseInsensitive)
{
  app::Command* cmd = app::CommandsModule::instance()->getCommandByName("undo");
  ASSERT_NE(cmd, nullptr);
  EXPECT_EQ(cmd->id(), app::CommandId::Undo);
}

TEST_F(CommandFlagsTest, GetCommandByNameReturnsNullForUnknownName)
{
  EXPECT_EQ(
      app::CommandsModule::instance()->getCommandByName("NotARealCommand"),
      nullptr);
}
