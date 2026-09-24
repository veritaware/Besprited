// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "ui/button.h"
#include "ui/timer.h"

#include <cstdint>

namespace app
{

// Status bar indicator for the MessageLog: hidden while the log is empty,
// otherwise shows the number of unread entries and opens the Messages window
// when clicked. Polls MessageLog::generation() because entries can be added
// from worker threads.
class MessageLogButton : public ui::Button
{
public:
  MessageLogButton();

protected:
  bool onProcessMessage(ui::Message* msg) override;
  void onSizeHint(ui::SizeHintEvent& ev) override;
  void onPaint(ui::PaintEvent& ev) override;
  void onClick(ui::Event& ev) override;

private:
  void update();
  std::string label() const;

  ui::Timer m_timer;
  std::uint64_t m_generation = 0;
  std::size_t m_unread = 0;
  bool m_hasErrors = false;
};

} // namespace app
