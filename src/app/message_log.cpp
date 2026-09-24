// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/message_log.h"

#include <algorithm>
#include <ctime>

namespace app
{

// static
MessageLog* MessageLog::instance()
{
  static MessageLog log;
  return &log;
}

MessageLog::MessageLog(std::size_t capacity)
  : m_capacity(std::max<std::size_t>(capacity, 1))
{
}

void MessageLog::add(Severity severity, const std::string& text)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_entries.push_back({severity, text, std::chrono::system_clock::now(), false});
  while (m_entries.size() > m_capacity)
    m_entries.pop_front();
  ++m_generation;
}

std::vector<MessageLog::Entry> MessageLog::entries() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return {m_entries.begin(), m_entries.end()};
}

std::size_t MessageLog::size() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_entries.size();
}

std::size_t MessageLog::unreadCount() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return std::count_if(m_entries.begin(), m_entries.end(),
                       [](const Entry& e) { return !e.read; });
}

bool MessageLog::hasUnreadErrors() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return std::any_of(m_entries.begin(), m_entries.end(),
                     [](const Entry& e)
                     { return !e.read && e.severity == Severity::Error; });
}

bool MessageLog::latest(Entry& out) const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (m_entries.empty())
    return false;
  out = m_entries.back();
  return true;
}

void MessageLog::markAllRead()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  bool changed = false;
  for (Entry& e : m_entries)
  {
    if (!e.read)
    {
      e.read = true;
      changed = true;
    }
  }
  if (changed)
    ++m_generation;
}

void MessageLog::clear()
{
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_entries.empty())
  {
    m_entries.clear();
    ++m_generation;
  }
}

std::uint64_t MessageLog::generation() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_generation;
}

std::string MessageLog::toText() const
{
  std::string out;
  for (const Entry& e : entries())
  {
    std::time_t t = std::chrono::system_clock::to_time_t(e.time);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    char stamp[16];
    std::strftime(stamp, sizeof(stamp), "%H:%M:%S", &tm);

    out += stamp;
    out += " [";
    out += severityName(e.severity);
    out += "] ";
    out += e.text;
    if (out.empty() || out.back() != '\n')
      out += '\n';
  }
  return out;
}

// static
const char* MessageLog::severityName(Severity severity)
{
  return severity == Severity::Error ? "Error" : "Warning";
}

} // namespace app
