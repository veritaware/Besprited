// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/editor/scrolling_state.h"

#include "app/app.h"
#include "app/ui/editor/editor.h"
#include "app/ui/status_bar.h"
#include "gfx/rect.h"
#include "doc/sprite.h"
#include "ui/message.h"
#include "ui/system.h"
#include "ui/view.h"

namespace app
{

using namespace ui;

ScrollingState::ScrollingState() = default;

ScrollingState::~ScrollingState() = default;

bool ScrollingState::onMouseDown(Editor* editor, MouseMessage* msg)
{
  beginDrag(editor, msg->position());
  return true;
}

// Scrolling works in screen space, not editor space.
gfx::Point ScrollingState::dragPos(Editor* editor, MouseMessage* msg) const
{
  return msg->position();
}

bool ScrollingState::skipDrag(Editor* editor, MouseMessage* msg)
{
#ifdef _WIN32
  gfx::Point newPos = msg->position();
  if (newPos != editor->autoScroll(msg, AutoScroll::ScrollDir))
  {
    rebaseDrag(newPos);
    return true;
  }
#endif
  return false;
}

void ScrollingState::onDrag(Editor* editor, const gfx::Point& delta)
{
  // The view clamps its scroll, so keep deltas incremental: apply this one
  // to the current scroll and make the current position the new start.
  gfx::Point scroll = View::getView(editor)->viewScroll();
  editor->setEditorScroll(scroll - delta);
  rebaseDrag(dragStart() + delta);
}

bool ScrollingState::onSetCursor(Editor* editor,
                                 const gfx::Point& mouseScreenPos)
{
  editor->showMouseCursor(kScrollCursor);
  return true;
}

bool ScrollingState::onKeyDown(Editor* editor, KeyMessage* msg)
{
  return false;
}

bool ScrollingState::onKeyUp(Editor* editor, KeyMessage* msg)
{
  return false;
}

bool ScrollingState::onUpdateStatusBar(Editor* editor)
{
  return false;
}

} // namespace app
