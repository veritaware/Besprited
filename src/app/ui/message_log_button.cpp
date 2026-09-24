// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/ui/message_log_button.h"

#include "app/commands/commands.h"
#include "app/message_log.h"
#include "app/ui/skin/skin_theme.h"
#include "app/ui/skin/style.h"
#include "app/ui/status_bar.h"
#include "app/ui_context.h"
#include "ui/message.h"
#include "ui/paint_event.h"
#include "ui/size_hint_event.h"
#include "ui/system.h"

namespace app
{

using namespace ui;

namespace
{

// Only the first line, so a multi-line error doesn't blow up the tip.
std::string first_line(const std::string& text)
{
  return text.substr(0, text.find('\n'));
}

// Matches the "x" offset of the icon in the message_log_button style.
constexpr int kIconOffset = 2;
constexpr int kIconTextGap = 4;

} // namespace

MessageLogButton::MessageLogButton()
  : Button("")
  , m_timer(250, this)
{
  setVisible(false);
  m_timer.start();
}

bool MessageLogButton::onProcessMessage(Message* msg)
{
  switch (msg->type())
  {
  case kTimerMessage:
    if (m_generation != MessageLog::instance()->generation())
      update();
    break;

  case kSetCursorMessage:
    ui::set_mouse_cursor(kHandCursor);
    return true;
  }
  return Button::onProcessMessage(msg);
}

void MessageLogButton::update()
{
  MessageLog* log = MessageLog::instance();

  // Read the generation first: an entry added while we look at the log
  // will bump it again and be picked up on the next tick.
  m_generation = log->generation();

  const std::size_t prevUnread = m_unread;
  m_unread = log->unreadCount();
  m_hasErrors = log->hasUnreadErrors();

  setVisible(log->size() > 0);
  if (parent())
    parent()->layout(); // the label width may have changed
  invalidate();

  // Briefly surface the new problem, like other status bar tips.
  MessageLog::Entry latest;
  if (m_unread > prevUnread && log->latest(latest) && StatusBar::instance())
  {
    StatusBar::instance()->showTip(2000, "%s: %s",
                                   MessageLog::severityName(latest.severity),
                                   first_line(latest.text).c_str());
  }
}

std::string MessageLogButton::label() const
{
  return m_unread > 0 ? std::to_string(m_unread) : std::string();
}

void MessageLogButton::onSizeHint(SizeHintEvent& ev)
{
  auto* theme = static_cast<skin::SkinTheme*>(this->theme());
  gfx::Size sz =
      theme->styles.messageLogButton()->sizeHint(nullptr, skin::Style::State());
  const std::string text = label();
  if (!text.empty())
    sz.w += (kIconOffset + kIconTextGap) * guiscale() +
            Graphics::measureUIStringLength(text, font().get());
  sz.w += 4 * guiscale();
  ev.setSizeHint(sz);
}

void MessageLogButton::onPaint(PaintEvent& ev)
{
  auto* theme = static_cast<skin::SkinTheme*>(this->theme());
  Graphics* g = ev.graphics();
  gfx::Rect rc = clientBounds();

  skin::Style::State state;
  if (hasMouseOver())
    state += skin::Style::hover();

  g->fillRect(bgColor(), rc);
  theme->styles.messageLogButton()->paint(g, rc, nullptr, state);

  const std::string text = label();
  if (!text.empty())
  {
    gfx::Color color = m_hasErrors ? theme->colors.flagActive()
                                   : theme->colors.statusBarText();
    const int iconW = theme->styles.messageLogButton()
                          ->sizeHint(nullptr, skin::Style::State())
                          .w;
    g->drawString(
        text, color, gfx::ColorNone,
        gfx::Point(rc.x + iconW + (kIconOffset + kIconTextGap) * guiscale(),
                   rc.y + rc.h / 2 - textHeight() / 2));
  }
}

void MessageLogButton::onClick(Event& ev)
{
  Button::onClick(ev);

  Command* cmd =
      CommandsModule::instance()->getCommandByName(CommandId::MessageLog);
  UIContext::instance()->executeCommand(cmd);
}

} // namespace app
