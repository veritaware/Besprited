// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/message_log.h"

#include <thread>
#include <vector>

using namespace app;
using Severity = MessageLog::Severity;

TEST(MessageLog, StartsEmpty)
{
  MessageLog log;
  EXPECT_EQ(0u, log.size());
  EXPECT_EQ(0u, log.unreadCount());
  EXPECT_FALSE(log.hasUnreadErrors());
  EXPECT_EQ("", log.toText());

  MessageLog::Entry e;
  EXPECT_FALSE(log.latest(e));
}

TEST(MessageLog, AddTracksUnreadAndLatest)
{
  MessageLog log;
  log.add(Severity::Warning, "first");
  log.add(Severity::Error, "second");

  EXPECT_EQ(2u, log.size());
  EXPECT_EQ(2u, log.unreadCount());
  EXPECT_TRUE(log.hasUnreadErrors());

  MessageLog::Entry e;
  ASSERT_TRUE(log.latest(e));
  EXPECT_EQ("second", e.text);
  EXPECT_EQ(Severity::Error, e.severity);
  EXPECT_FALSE(e.read);

  auto all = log.entries();
  ASSERT_EQ(2u, all.size());
  EXPECT_EQ("first", all[0].text); // oldest first
}

TEST(MessageLog, MarkAllReadKeepsEntries)
{
  MessageLog log;
  log.add(Severity::Error, "boom");
  log.markAllRead();

  EXPECT_EQ(1u, log.size());
  EXPECT_EQ(0u, log.unreadCount());
  EXPECT_FALSE(log.hasUnreadErrors());

  log.add(Severity::Warning, "later");
  EXPECT_EQ(1u, log.unreadCount());
  EXPECT_FALSE(log.hasUnreadErrors()); // the old error stays read
}

TEST(MessageLog, ClearEmptiesTheLog)
{
  MessageLog log;
  log.add(Severity::Warning, "a");
  log.clear();
  EXPECT_EQ(0u, log.size());
  EXPECT_EQ(0u, log.unreadCount());
}

TEST(MessageLog, CapacityDropsOldestEntries)
{
  MessageLog log(3);
  for (int i = 0; i < 5; ++i)
    log.add(Severity::Warning, "m" + std::to_string(i));

  auto all = log.entries();
  ASSERT_EQ(3u, all.size());
  EXPECT_EQ("m2", all[0].text);
  EXPECT_EQ("m4", all[2].text);
}

TEST(MessageLog, GenerationChangesOnlyWhenContentChanges)
{
  MessageLog log;
  auto g0 = log.generation();

  log.markAllRead(); // nothing to mark
  log.clear();       // nothing to clear
  EXPECT_EQ(g0, log.generation());

  log.add(Severity::Warning, "a");
  auto g1 = log.generation();
  EXPECT_NE(g0, g1);

  log.markAllRead();
  auto g2 = log.generation();
  EXPECT_NE(g1, g2);

  log.markAllRead(); // already read
  EXPECT_EQ(g2, log.generation());

  log.clear();
  EXPECT_NE(g2, log.generation());
}

TEST(MessageLog, ToTextListsSeverityAndText)
{
  MessageLog log;
  log.add(Severity::Warning, "careful");
  log.add(Severity::Error, "broke\n");

  const std::string text = log.toText();
  EXPECT_NE(std::string::npos, text.find("[Warning] careful\n"));
  EXPECT_NE(std::string::npos, text.find("[Error] broke\n"));
  EXPECT_EQ(std::string::npos, text.find("broke\n\n"));
}

TEST(MessageLog, ConcurrentAddsAreAllRecorded)
{
  MessageLog log(1000);
  std::vector<std::thread> threads;
  for (int t = 0; t < 4; ++t)
    threads.emplace_back(
        [&log]
        {
          for (int i = 0; i < 100; ++i)
            log.add(Severity::Warning, "x");
        });
  for (auto& th : threads)
    th.join();

  EXPECT_EQ(400u, log.size());
  EXPECT_EQ(400u, log.unreadCount());
}
