// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tests.h"

#include "doc/color.h"
#include "doc/palette.h"

using namespace doc;

// File-format loaders pass indices read straight from the file into
// Palette::setEntry() (issue #219), so it must reject out-of-range indices
// with a real check rather than a compiled-out ASSERT.
TEST(Palette, SetEntryIgnoresOutOfRangeIndex)
{
  const color_t original = rgba(1, 2, 3, 255);
  const color_t other = rgba(0x41, 0x41, 0x41, 0xFF);

  auto pal = Palette::create(2);
  pal->setEntry(0, original);
  pal->setEntry(1, original);
  const int mods = pal->getModifications();

  pal->setEntry(-1, other);
  pal->setEntry(2, other);
  pal->setEntry(1 << 20, other);

  EXPECT_EQ(2, pal->size());
  EXPECT_EQ(original, pal->getEntry(0));
  EXPECT_EQ(original, pal->getEntry(1));
  EXPECT_EQ(mods, pal->getModifications());
}

TEST(Palette, SetEntryAcceptsInRangeIndex)
{
  auto pal = Palette::create(2);
  const color_t c = rgba(9, 8, 7, 255);
  const int mods = pal->getModifications();

  pal->setEntry(1, c);

  EXPECT_EQ(c, pal->getEntry(1));
  EXPECT_EQ(mods + 1, pal->getModifications());
}

TEST(Palette, EnsureTransparentFirstInsertsAndShifts)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(1, 2, 3, 255));
  pal->setEntry(1, rgba(4, 5, 6, 255));
  pal->setEntryName(1, "second");

  EXPECT_TRUE(pal->ensureTransparentFirst());
  ASSERT_EQ(3, pal->size());
  EXPECT_EQ(rgba(0, 0, 0, 0), pal->getEntry(0));
  EXPECT_EQ(rgba(1, 2, 3, 255), pal->getEntry(1));
  EXPECT_EQ(rgba(4, 5, 6, 255), pal->getEntry(2));
  EXPECT_EQ("second", pal->entryName(2));
}

TEST(Palette, EnsureTransparentFirstMovesExisting)
{
  auto pal = Palette::create(3);
  pal->setEntry(0, rgba(1, 2, 3, 255));
  pal->setEntry(1, rgba(9, 9, 9, 0));
  pal->setEntry(2, rgba(4, 5, 6, 255));

  EXPECT_TRUE(pal->ensureTransparentFirst());
  ASSERT_EQ(3, pal->size());
  EXPECT_EQ(rgba(9, 9, 9, 0), pal->getEntry(0));
  EXPECT_EQ(rgba(1, 2, 3, 255), pal->getEntry(1));
  EXPECT_EQ(rgba(4, 5, 6, 255), pal->getEntry(2));
}

TEST(Palette, EnsureTransparentFirstNoopWhenAlreadyFirst)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(0, 0, 0, 0));
  pal->setEntry(1, rgba(4, 5, 6, 255));
  EXPECT_FALSE(pal->ensureTransparentFirst());
  EXPECT_EQ(rgba(4, 5, 6, 255), pal->getEntry(1));
}

TEST(Palette, EnsureTransparentFirstRespectsMaxColors)
{
  auto pal = Palette::create(256);
  for (int i = 0; i < 256; ++i)
    pal->setEntry(i, rgba(i, 0, 0, 255));

  EXPECT_TRUE(pal->ensureTransparentFirst());
  ASSERT_EQ(256, pal->size());
  EXPECT_EQ(rgba(0, 0, 0, 0), pal->getEntry(0));
  EXPECT_EQ(rgba(0, 0, 0, 255), pal->getEntry(1));
  EXPECT_EQ(rgba(254, 0, 0, 255), pal->getEntry(255));
}
