// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/context_access.h"
#include "app/context_flags.h"
#include "app/document.h"
#include "app/document_api.h"
#include "app/transaction.h"

#include <cstdint>
#include <string>
#include <utility>

// Small scaffolding shared by app/commands/cmd_*.cpp: a couple of the
// onEnabled() ContextFlags combos that get repeated verbatim across many
// commands, and a helper for the ContextWriter+Transaction+DocumentApi
// dance most onExecute() bodies open with. See issue #228.
//
// Deliberately NOT rolled out as a sweep: migrate an existing cmd_*.cpp to
// these only when you're touching that file for another reason anyway - see
// the issue for why a dedicated migration PR isn't worth the diff.

namespace app::cmd_flags
{

// checkFlags() combo for a command that needs a writable document with an
// active sprite, but doesn't care about the layer/selection beyond that
// (e.g. CanvasSize, Crop-by-params, RotateCanvas).
inline constexpr uint32_t WritableSprite =
    ContextFlags::ActiveDocumentIsWritable | ContextFlags::HasActiveSprite;

// checkFlags() combo for a command that paints/erases into the active
// cel's image, bounded by the current selection (Fill, Stroke, ...).
inline constexpr uint32_t EditableCel =
    ContextFlags::ActiveDocumentIsWritable |
    ContextFlags::ActiveLayerIsEditable | ContextFlags::ActiveLayerIsImage |
    ContextFlags::HasVisibleMask;

} // namespace app::cmd_flags

namespace app
{

// Opens a writable context, starts a Transaction with the given label,
// hands `fn` a DocumentApi to make edits through, and commits on return.
// Equivalent to the ContextWriter/Transaction/DocumentApi boilerplate
// repeated across most Command::onExecute() bodies:
//
//   ContextWriter writer(context);
//   Transaction transaction(writer.context(), label);
//   DocumentApi api = writer.document()->getApi(transaction);
//   ... fn body ...
//   transaction.commit();
//
// `fn` is called as fn(writer, transaction, api).
template <class Fn>
void with_transaction(Context* ctx, const std::string& label, Fn&& fn)
{
  ContextWriter writer(ctx);
  Transaction transaction(writer.context(), label);
  DocumentApi api = writer.document()->getApi(transaction);
  std::forward<Fn>(fn)(writer, transaction, api);
  transaction.commit();
}

} // namespace app
