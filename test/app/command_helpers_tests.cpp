// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/app.h"
#include "app/commands/command_helpers.h"
#include "app/document.h"
#include "app/document_undo.h"
#include "doc/sprite.h"
#include "doc/test_context.h"

namespace
{

// See test/app/fill_stroke_mask_tests.cpp:35-51 for why this is deliberately
// leaked and constructed at most once for the whole test binary.
void ensureApp()
{
  static app::App* app = new app::App();
  app->initializeCoreModulesForTesting();
}

// Same fixture pattern as fill_stroke_mask_tests.cpp: a TestContextT keeps
// the just-added document active, which is what with_transaction()'s
// ContextWriter needs to find an active document/sprite without a real UI
// selecting one.
struct CommandHelpersFixture : public ::testing::Test
{
  doc::TestContextT<app::Context> ctx;
  app::Document* doc = nullptr;

  CommandHelpersFixture() { ensureApp(); }

  void makeSprite(int w, int h)
  {
    doc = static_cast<app::Document*>(ctx.documents().add(w, h));
  }
};

} // namespace

TEST_F(CommandHelpersFixture, WithTransactionAppliesTheEditAndCommits)
{
  makeSprite(4, 4);

  app::with_transaction(
      &ctx, "Resize",
      [](app::ContextWriter& writer, app::Transaction&, app::DocumentApi& api)
      { api.setSpriteSize(writer.sprite(), 8, 6); });

  EXPECT_EQ(8, doc->sprite()->width());
  EXPECT_EQ(6, doc->sprite()->height());
}

TEST_F(CommandHelpersFixture, WithTransactionEditIsUndoable)
{
  makeSprite(4, 4);

  app::with_transaction(
      &ctx, "Resize",
      [](app::ContextWriter& writer, app::Transaction&, app::DocumentApi& api)
      { api.setSpriteSize(writer.sprite(), 10, 10); });

  ASSERT_EQ(10, doc->sprite()->width());
  ASSERT_TRUE(doc->undoHistory()->canUndo());

  doc->undoHistory()->undo();

  EXPECT_EQ(4, doc->sprite()->width());
  EXPECT_EQ(4, doc->sprite()->height());
}

TEST(CommandFlags, WritableSpriteCombinesTheExpectedBits)
{
  EXPECT_EQ(app::ContextFlags::ActiveDocumentIsWritable |
                app::ContextFlags::HasActiveSprite,
            app::cmd_flags::WritableSprite);
}

TEST(CommandFlags, EditableCelCombinesTheExpectedBits)
{
  EXPECT_EQ(app::ContextFlags::ActiveDocumentIsWritable |
                app::ContextFlags::ActiveLayerIsEditable |
                app::ContextFlags::ActiveLayerIsImage |
                app::ContextFlags::HasVisibleMask,
            app::cmd_flags::EditableCel);
}
