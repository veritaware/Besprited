// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include <exception>

namespace app
{
class Context;

class Console
{
public:
  Console(Context* ctx = nullptr);
  ~Console();

  // Shows the text in the modal Errors Console (or stdout without UI).
  // Not recorded in the MessageLog: it's also the script console's sink.
  void printf(const char* format, ...);

  // Non-critical problem: recorded in the MessageLog only (no modal
  // window). Without UI it is also printed to stdout.
  void warning(const char* format, ...);

  // Critical problem: shown as with printf() and also recorded in the
  // MessageLog.
  void error(const char* format, ...);

  static void showException(const std::exception& e);

private:
  bool m_withUI;
};

} // namespace app
