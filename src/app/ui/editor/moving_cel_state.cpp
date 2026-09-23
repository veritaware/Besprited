// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/editor/moving_cel_state.h"

#include "app/app.h"
#include "app/context_access.h"
#include "app/document_api.h"
#include "app/document_range.h"
#include "app/ui/editor/editor.h"
#include "app/ui/editor/editor_customization_delegate.h"
#include "app/ui/main_window.h"
#include "app/ui/status_bar.h"
#include "app/ui/timeline.h"
#include "app/ui_context.h"
#include "app/transaction.h"
#include "app/util/range_utils.h"
#include "doc/cel.h"
#include "doc/layer.h"
#include "doc/mask.h"
#include "doc/sprite.h"
#include "ui/message.h"

namespace app
{

using namespace ui;

MovingCelState::MovingCelState(Editor* editor, MouseMessage* msg)
  : m_reader(UIContext::instance(), 500)
  , m_canceled(false)
{
  ContextWriter writer(m_reader);
  Document* document = editor->document();
  auto range = App::instance()->timeline()->range();
  LayerImage* layer = static_cast<LayerImage*>(editor->layer());
  ASSERT(layer->isImage());

  auto currentCel = layer->cel(editor->frame());
  ASSERT(currentCel); // The cel cannot be null

  if (!range.enabled())
    range = DocumentRange(currentCel.get());

  // Record start positions of all cels in selected range
  for (auto cel : get_unique_cels(writer.sprite(), range))
  {
    Layer* layer = cel->layer();
    ASSERT(layer);

    if (layer && layer->isMovable() && !layer->isBackground())
    {
      m_celList.push_back(cel);
      m_celStarts.push_back(cel->position());
    }
  }

  beginDrag(editor, editor->screenToEditor(msg->position()));
  m_celOffset = gfx::Point(0, 0);

  // Hide the mask (temporarily, until mouse-up event)
  m_maskVisible = document->isMaskVisible();
  if (m_maskVisible)
  {
    document->setMaskVisible(false);
    document->generateMaskBoundaries();
  }
}

MovingCelState::~MovingCelState() = default;

void MovingCelState::onDragEnd(Editor* editor)
{
  Document* document = editor->document();

  // Here we put back the cel into its original coordinate (so we can
  // add an undoer before).
  if (m_celOffset != gfx::Point(0, 0))
  {
    // Put the cels in the original position.
    for (size_t i = 0; i < m_celList.size(); ++i)
    {
      auto cel = m_celList[i];
      const gfx::Point& celStart = m_celStarts[i];

      cel->setPosition(celStart);
    }

    // If the user didn't cancel the operation...
    if (!m_canceled)
    {
      ContextWriter writer(m_reader);
      Transaction transaction(writer.context(), "Cel Movement", ModifyDocument);
      DocumentApi api = document->getApi(transaction);

      // And now we move the cel (or all selected range) to the new position.
      for (auto cel : m_celList)
      {
        api.setCelPosition(writer.sprite(), cel, cel->x() + m_celOffset.x,
                           cel->y() + m_celOffset.y);
      }

      // Move selection if it was visible
      if (m_maskVisible)
        api.setMaskPosition(document->mask()->bounds().x + m_celOffset.x,
                            document->mask()->bounds().y + m_celOffset.y);

      transaction.commit();
    }

    // Redraw all editors. We've to notify all views about this
    // general update because MovingCelState::onDrag() redraws only the
    // cels in the current editor. And at this point we'd like to update
    // all the editors.
    document->notifyGeneralUpdate();
  }

  // Restore the mask visibility.
  if (m_maskVisible)
  {
    document->setMaskVisible(m_maskVisible);
    document->generateMaskBoundaries();
  }
}

void MovingCelState::onDrag(Editor* editor, const gfx::Point& delta)
{
  m_celOffset = delta;

  if (int(editor->getCustomizationDelegate()->getPressedKeyAction(
              KeyContext::TranslatingSelection) &
          KeyAction::LockAxis))
  {
    if (ABS(m_celOffset.x) < ABS(m_celOffset.y))
    {
      m_celOffset.x = 0;
    }
    else
    {
      m_celOffset.y = 0;
    }
  }

  for (size_t i = 0; i < m_celList.size(); ++i)
  {
    auto cel = m_celList[i];
    const gfx::Point& celStart = m_celStarts[i];

    cel->setPosition(celStart + m_celOffset);
  }

  // Redraw the new cel position.
  editor->invalidate();
}

bool MovingCelState::onUpdateStatusBar(Editor* editor)
{
  StatusBar::instance()->setStatusText(
      0, ":pos: %3d %3d :offset: %3d %3d", (int)dragStart().x,
      (int)dragStart().y, (int)m_celOffset.x, (int)m_celOffset.y);

  return true;
}

} // namespace app
