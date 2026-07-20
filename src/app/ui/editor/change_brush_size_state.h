// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "app/ui/editor/editor_state.h"
#include "gfx/point.h"

namespace app {

  namespace tools {
    class Tool;
  }

  // Lets the user drag the mouse (while holding the "Change Brush
  // Size" action modifier, e.g. Alt+Right-click) to change the
  // active tool's brush size (horizontal movement) and, when the
  // tool's ink supports it, its opacity (vertical movement).
  class ChangeBrushSizeState : public EditorState {
  public:
    ChangeBrushSizeState();
    virtual ~ChangeBrushSizeState();
    virtual bool isTemporalState() const override { return true; }
    virtual bool onMouseDown(Editor* editor, ui::MouseMessage* msg) override;
    virtual bool onMouseUp(Editor* editor, ui::MouseMessage* msg) override;
    virtual bool onMouseMove(Editor* editor, ui::MouseMessage* msg) override;
    virtual bool onSetCursor(Editor* editor, const gfx::Point& mouseScreenPos) override;
    virtual bool onKeyDown(Editor* editor, ui::KeyMessage* msg) override;
    virtual bool onKeyUp(Editor* editor, ui::KeyMessage* msg) override;
    virtual bool onUpdateStatusBar(Editor* editor) override;

  private:
    tools::Tool* m_tool;
    gfx::Point m_startPos;
    int m_startSize;
    int m_startOpacity;
    bool m_changeOpacity;
  };

} // namespace app
