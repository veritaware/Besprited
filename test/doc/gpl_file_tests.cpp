// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tests.h"

#include "doc/file/gpl_file.h"
#include "doc/palette.h"

#include <cstdio>
#include <fstream>

using namespace doc;

namespace
{

// Removes the test file even when a failed ASSERT_* returns early.
struct TempFile
{
  const char* path;
  explicit TempFile(const char* path, const char* contents = nullptr)
    : path(path)
  {
    if (contents)
      std::ofstream(path) << contents;
  }
  ~TempFile() { std::remove(path); }
};

} // namespace

TEST(GplFile, ReadsPaletteNameAuthorAndColorNames)
{
  const TempFile tmp("gpl_meta_test.gpl", "GIMP Palette\n"
                                          "#Palette Name: Test Pal\n"
                                          "#Author: Jane Doe\n"
                                          "#Colors: 3\n"
                                          "  1   2   3\tSky Blue\n"
                                          "  4   5   6\tUntitled\n"
                                          "  7   8   9\n");

  auto pal = file::load_gpl_file(tmp.path);
  ASSERT_TRUE(pal != nullptr);
  ASSERT_EQ(3, pal->size());
  EXPECT_EQ("Test Pal", pal->name());
  EXPECT_EQ("Jane Doe", pal->author());
  EXPECT_EQ(rgba(1, 2, 3, 255), pal->entry(0));
  EXPECT_EQ("Sky Blue", pal->entryName(0));
  EXPECT_EQ("", pal->entryName(1));
  EXPECT_EQ("", pal->entryName(2));
}

TEST(GplFile, ReadsLegacyByAndNameHeaders)
{
  const TempFile tmp("gpl_legacy_test.gpl", "GIMP Palette\n"
                                            "Name: Old Style\n"
                                            "#\n"
                                            "# by Richard Fhager\n"
                                            "# http://example.com\n"
                                            "#\n"
                                            "  0   0   0\tblack\n");

  auto pal = file::load_gpl_file(tmp.path);
  ASSERT_TRUE(pal != nullptr);
  EXPECT_EQ("Old Style", pal->name());
  EXPECT_EQ("Richard Fhager", pal->author());
  EXPECT_EQ("black", pal->entryName(0));
}

TEST(GplFile, SaveLoadRoundTripKeepsMetadataAndNames)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(10, 20, 30, 255));
  pal->setEntry(1, rgba(40, 50, 60, 255));
  pal->setName("RT");
  pal->setAuthor("Me");
  pal->setEntryName(1, "Named Two");

  const TempFile tmp("gpl_roundtrip_test.gpl");
  ASSERT_TRUE(file::save_gpl_file(*pal, tmp.path));

  auto loaded = file::load_gpl_file(tmp.path);
  ASSERT_TRUE(loaded != nullptr);
  ASSERT_EQ(2, loaded->size());
  EXPECT_EQ("RT", loaded->name());
  EXPECT_EQ("Me", loaded->author());
  EXPECT_EQ("", loaded->entryName(0));
  EXPECT_EQ("Named Two", loaded->entryName(1));
  EXPECT_EQ(rgba(40, 50, 60, 255), loaded->entry(1));
}

TEST(GplFile, SaveSanitizesNamesSoTheFileReloadsIntact)
{
  auto pal = Palette::create(1);
  pal->setName("A\nB");
  pal->setEntryName(0, "x\n255 0 0 evil");

  const TempFile tmp("gpl_sanitize_test.gpl");
  ASSERT_TRUE(file::save_gpl_file(*pal, tmp.path));

  auto loaded = file::load_gpl_file(tmp.path);
  ASSERT_TRUE(loaded != nullptr);
  EXPECT_EQ(1, loaded->size());
  EXPECT_EQ("A B", loaded->name());
}

TEST(GplFile, ByCommentAfterColorsIsNotAuthor)
{
  const TempFile tmp("gpl_by_test.gpl", "GIMP Palette\n"
                                        "  0   0   0\tblack\n"
                                        "# by hand\n");

  auto pal = file::load_gpl_file(tmp.path);
  ASSERT_TRUE(pal != nullptr);
  EXPECT_EQ("", pal->author());
}

TEST(Palette, NameOrAuthorOnlyDifferenceCountsWithNames)
{
  auto a = Palette::create(2);
  auto b = a->clone();
  b->setAuthor("Someone");
  int from, to;
  EXPECT_EQ(0, a->countDiff(*b, nullptr, nullptr));
  EXPECT_EQ(1, a->countDiff(*b, &from, &to, true));
  EXPECT_EQ(0, from);
  EXPECT_EQ(0, to);
}
