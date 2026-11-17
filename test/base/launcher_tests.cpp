// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <gtest/gtest.h>

#include "base/launcher.h"

using namespace base::launcher;

TEST(Launcher, IsWebUrlAcceptsOnlyHttpAndHttps)
{
  EXPECT_TRUE(is_web_url("http://example.com"));
  EXPECT_TRUE(is_web_url("https://example.com/a?b=c"));
  EXPECT_TRUE(is_web_url("HTTPS://example.com"));
}

TEST(Launcher, IsWebUrlRejectsEverythingElse)
{
  EXPECT_FALSE(is_web_url(""));
  EXPECT_FALSE(is_web_url("https"));
  EXPECT_FALSE(is_web_url("https:"));
  EXPECT_FALSE(is_web_url("file:///tmp/a.sh"));
  EXPECT_FALSE(is_web_url("ftp://example.com"));
  EXPECT_FALSE(is_web_url("C:\\Windows\\System32\\cmd.exe"));
  EXPECT_FALSE(is_web_url("/usr/bin/xdg-open"));
  EXPECT_FALSE(is_web_url("\\\\server\\share\\a.exe"));
}
