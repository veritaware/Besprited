// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/editor/change_brush_size_state.h"

#include "app/app.h"
#include "app/pref/preferences.h"
#include "app/tools/ink.h"
#include "app/tools/ink_type.h"
#include "app/tools/tool.h"
#include "app/ui/editor/editor.h"
#include "app/ui/status_bar.h"
#include "doc/brush.h"
#include "ui/message.h"

#include <algorithm>

namespace app {

using namespace ui;

ChangeBrushSizeState::ChangeBrushSizeState()
  : m_tool(nullptr)
  , m_startSize(doc::Brush::kMinBrushSize)
  , m_startOpacity(0)
  , m_changeOpacity(false)
{
}

ChangeBrushSizeState::~ChangeBrushSizeState()
{
}

bool ChangeBrushSizeState::onMouseDown(Editor* editor, MouseMessage* msg)
{
  m_startPos = msg->position();
  m_tool = App::instance()->activeTool();

  ToolPreferences& toolPref = Preferences::instance().tool(m_tool);
  m_startSize = toolPref.brush.size();
  m_startOpacity = toolPref.opacity();

  bool isPaint = m_tool->getInk(0)->isPaint() || m_tool->getInk(1)->isPaint();
  bool isEffect = m_tool->getInk(0)->isEffect() || m_tool->getInk(1)->isEffect();

  // The Y axis changes the opacity for tools whose ink supports it
  // (e.g. Alpha Compositing) and for effect-based tools that always
  // have an opacity value of their own (e.g. the Eraser).
  m_changeOpacity =
    isEffect || (isPaint && tools::inkHasOpacity(toolPref.ink()));

  editor->captureMouse();
  return true;
}

bool ChangeBrushSizeState::onMouseUp(Editor* editor, MouseMessage* msg)
{
  editor->backToPreviousState();
  editor->releaseMouse();
  return true;
}

bool ChangeBrushSizeState::onMouseMove(Editor* editor, MouseMessage* msg)
{
  if (!m_tool)
    return true;

  gfx::Point delta = msg->position() - m_startPos;
  ToolPreferences& toolPref = Preferences::instance().tool(m_tool);

  int newSize = std::max(doc::Brush::kMinBrushSize,
    std::min(doc::Brush::kMaxBrushSize, m_startSize + delta.x));
  toolPref.brush.size(newSize);

  if (m_changeOpacity) {
    int newOpacity = std::max(0, std::min(255, m_startOpacity - delta.y));
    toolPref.opacity(newOpacity);
  }

  editor->updateStatusBar();
  return true;
}

bool ChangeBrushSizeState::onSetCursor(Editor* editor, const gfx::Point& mouseScreenPos)
{
  editor->showMouseCursor(kSizeWECursor);
  return true;
}

bool ChangeBrushSizeState::onKeyDown(Editor* editor, KeyMessage* msg)
{
  return false;
}

bool ChangeBrushSizeState::onKeyUp(Editor* editor, KeyMessage* msg)
{
  return false;
}

bool ChangeBrushSizeState::onUpdateStatusBar(Editor* editor)
{
  if (!m_tool)
    return false;

  ToolPreferences& toolPref = Preferences::instance().tool(m_tool);

  if (m_changeOpacity) {
    StatusBar::instance()->setStatusText(0,
      "Brush Size: %d, Opacity: %d",
      toolPref.brush.size(), toolPref.opacity());
  }
  else {
    StatusBar::instance()->setStatusText(0,
      "Brush Size: %d", toolPref.brush.size());
  }

  return true;
}

} // namespace app
