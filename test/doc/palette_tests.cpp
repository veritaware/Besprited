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
