// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/ui/editor/drag_state.h"
#include "app/ui/editor/editor_state.h"
#include "gfx/point.h"

namespace app
{

class ScrollingState : public DragState<EditorState>
{
public:
  ScrollingState();
  virtual ~ScrollingState();
  virtual bool isTemporalState() const override { return true; }
  virtual bool onMouseDown(Editor* editor, ui::MouseMessage* msg) override;
  virtual bool onSetCursor(Editor* editor,
                           const gfx::Point& mouseScreenPos) override;
  virtual bool onKeyDown(Editor* editor, ui::KeyMessage* msg) override;
  virtual bool onKeyUp(Editor* editor, ui::KeyMessage* msg) override;
  virtual bool onUpdateStatusBar(Editor* editor) override;

protected:
  gfx::Point dragPos(Editor* editor, ui::MouseMessage* msg) const override;
  bool skipDrag(Editor* editor, ui::MouseMessage* msg) override;
  void onDrag(Editor* editor, const gfx::Point& delta) override;
};

} // namespace app
