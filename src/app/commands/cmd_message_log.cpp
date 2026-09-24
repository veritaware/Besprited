// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/commands/command.h"
#include "app/message_log.h"
#include "app/modules/gui.h"
#include "clip/clip.h"
#include "ui/ui.h"

#include "messages.xml.h"

#include <climits>
#include <memory>

namespace app
{

class MessageLogWindow : public app::gen::Messages
{
public:
  MessageLogWindow()
    : m_textbox(new ui::TextBox("", ui::WORDWRAP))
    , m_timer(500, this)
  {
    view()->attachToView(m_textbox);

    copy()->Click.connect([this](ui::Event&)
                          { clip::set_text(app::MessageLog::instance()->toText()); });
    clear()->Click.connect(
        [this](ui::Event&)
        {
          app::MessageLog::instance()->clear();
          refresh();
        });
  }

private:
  bool onProcessMessage(ui::Message* msg) override
  {
    switch (msg->type())
    {
    case ui::kOpenMessage:
      load_window_pos(this, "MessageLog");
      refresh();
      m_timer.start();
      break;

    case ui::kCloseMessage:
      save_window_pos(this, "MessageLog");
      m_timer.stop();
      break;

    case ui::kTimerMessage:
      // The window is open, so whatever arrives is being looked at.
      if (m_generation != app::MessageLog::instance()->generation())
        refresh();
      break;
    }
    return app::gen::Messages::onProcessMessage(msg);
  }

  void refresh()
  {
    auto* log = app::MessageLog::instance();
    log->markAllRead();
    m_generation = log->generation();

    std::string text = log->toText();
    if (text.empty())
      text = "No warnings or errors in this session.";
    m_textbox->setText(text);

    view()->updateView();
    view()->setViewScroll(gfx::Point(0, INT_MAX)); // newest at the bottom
  }

  ui::TextBox* m_textbox;
  ui::Timer m_timer;
  std::uint64_t m_generation = 0;
};

class MessageLogCommand : public Command
{
public:
  MessageLogCommand();

protected:
  void onExecute(Context* ctx) override;
};

static MessageLogWindow* g_window = nullptr;

MessageLogCommand::MessageLogCommand()
  : Command("MessageLog", "Messages", CmdUIOnlyFlag)
{
}

void MessageLogCommand::onExecute(Context* ctx)
{
  if (!g_window)
    g_window = new MessageLogWindow();

  if (g_window->isVisible())
    g_window->closeWindow(nullptr);
  else
    g_window->openWindow();
}

std::unique_ptr<Command> CommandFactory::createMessageLogCommand()
{
  return std::make_unique<MessageLogCommand>();
}

} // namespace app
