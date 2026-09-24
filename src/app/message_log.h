// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

namespace app
{

// In-memory, per-session history of non-critical warnings and errors.
//
// Thread-safe: file operations and filters run on worker threads and report
// their problems from there. UI code must not be driven directly from add();
// poll generation() from the UI thread instead and refresh when it changes.
class MessageLog
{
public:
  enum class Severity
  {
    Warning,
    Error
  };

  struct Entry
  {
    Severity severity;
    std::string text;
    std::chrono::system_clock::time_point time;
    bool read;
  };

  static constexpr std::size_t kDefaultCapacity = 200;

  static MessageLog* instance();

  explicit MessageLog(std::size_t capacity = kDefaultCapacity);

  void add(Severity severity, const std::string& text);

  // Snapshot of all entries, oldest first.
  std::vector<Entry> entries() const;
  std::size_t size() const;
  std::size_t unreadCount() const;
  bool hasUnreadErrors() const;

  // Latest entry, or false if the log is empty.
  bool latest(Entry& out) const;

  void markAllRead();
  void clear();

  // Increases on every change (add/markAllRead/clear).
  std::uint64_t generation() const;

  // Plain-text dump of the whole log, e.g. for the clipboard.
  std::string toText() const;

  static const char* severityName(Severity severity);

private:
  mutable std::mutex m_mutex;
  std::size_t m_capacity;
  std::deque<Entry> m_entries;
  std::uint64_t m_generation = 0;
};

} // namespace app
