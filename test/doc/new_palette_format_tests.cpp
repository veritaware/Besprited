// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tests.h"

#include "doc/color.h"
#include "doc/file/act_file.h"
#include "doc/file/hex_file.h"
#include "doc/file/paintnet_file.h"
#include "doc/palette.h"

#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <system_error>
#include <vector>

using namespace doc;

namespace
{

struct TempFile
{
  std::filesystem::path directory;
  std::string path;

  explicit TempFile(const char* filename)
  {
    std::random_device random;
    directory = std::filesystem::temp_directory_path() /
                ("besprited_palette_tests_" + std::to_string(random()) + "_" + std::to_string(random()));
    std::filesystem::create_directories(directory);
    path = (directory / filename).string();
  }

  ~TempFile()
  {
    std::error_code error;
    std::filesystem::remove_all(directory, error);
  }
};

bool write_bytes(const char* path, const std::vector<uint8_t>& bytes)
{
  std::ofstream f(path, std::ios::binary);
  f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
  return f.good();
}

std::vector<uint8_t> sample_act(uint16_t count, uint16_t transparentIndex)
{
  std::vector<uint8_t> bytes(256 * 3, 0);
  bytes[0] = 0x12;
  bytes[1] = 0x34;
  bytes[2] = 0x56;
  bytes[3] = 0xab;
  bytes[4] = 0xcd;
  bytes[5] = 0xef;
  bytes.push_back(static_cast<uint8_t>(count >> 8));
  bytes.push_back(static_cast<uint8_t>(count & 0xff));
  bytes.push_back(static_cast<uint8_t>(transparentIndex >> 8));
  bytes.push_back(static_cast<uint8_t>(transparentIndex & 0xff));
  return bytes;
}

} // namespace

TEST(HexFile, ReadsRealLospecEndesga16Download)
{
  // Downloaded from https://lospec.com/palette-list/endesga-16.hex
  const std::filesystem::path fixture = std::filesystem::path(BESPRITED_PALETTE_FIXTURE_DIR) / "endesga-16.hex";
  const std::string path = fixture.string();
  auto pal = file::load_hex_file(path.c_str());
  ASSERT_NE(nullptr, pal);
  ASSERT_EQ(16, pal->size());
  EXPECT_EQ(rgba(0xe4, 0xa6, 0x72, 255), pal->entry(0));
  EXPECT_EQ(rgba(0x04, 0x84, 0xd1, 255), pal->entry(15));
}

TEST(HexFile, ReadsBomAndRgbaLines)
{
  const TempFile tmp("hex_bom_test.hex");
  std::ofstream(tmp.path, std::ios::binary) << "\xEF\xBB\xBF#AAbbCC\r\n\n#12345680\r\n";
  auto pal = file::load_hex_file(tmp.path.c_str());
  ASSERT_NE(nullptr, pal);
  ASSERT_EQ(2, pal->size());
  EXPECT_EQ(rgba(0xaa, 0xbb, 0xcc, 255), pal->entry(0));
  EXPECT_EQ(rgba(0x12, 0x34, 0x56, 0x80), pal->entry(1));
}

TEST(HexFile, SaveLoadRoundTripKeepsAlphaAndUsesRgba)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(0x12, 0x34, 0x56, 255));
  pal->setEntry(1, rgba(0xab, 0xcd, 0xef, 0x40));
  const TempFile tmp("hex_alpha_roundtrip.hex");

  ASSERT_TRUE(file::save_hex_file(*pal, tmp.path.c_str()));
  std::ifstream in(tmp.path, std::ios::binary);
  const std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  EXPECT_EQ("123456FF\nABCDEF40\n", contents);

  auto loaded = file::load_hex_file(tmp.path.c_str());
  ASSERT_NE(nullptr, loaded);
  ASSERT_EQ(2, loaded->size());
  EXPECT_EQ(pal->entry(0), loaded->entry(0));
  EXPECT_EQ(pal->entry(1), loaded->entry(1));
}

TEST(HexFile, SavesRgbWhenEveryEntryIsOpaque)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(0x12, 0x34, 0x56, 255));
  pal->setEntry(1, rgba(0xab, 0xcd, 0xef, 255));
  const TempFile tmp("hex_rgb_roundtrip.hex");

  ASSERT_TRUE(file::save_hex_file(*pal, tmp.path.c_str()));
  std::ifstream in(tmp.path, std::ios::binary);
  const std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  EXPECT_EQ("123456\nABCDEF\n", contents);
}

TEST(HexFile, RejectsOddLengthAndNonHexLines)
{
  const TempFile tmp("hex_invalid_test.hex");
  std::ofstream(tmp.path.c_str()) << "12345\n";
  EXPECT_EQ(nullptr, file::load_hex_file(tmp.path.c_str()));
  std::ofstream(tmp.path.c_str()) << "12345G\n";
  EXPECT_EQ(nullptr, file::load_hex_file(tmp.path.c_str()));
}

TEST(PaintNetFile, ReadsArgbEntriesAndSkipsComments)
{
  const TempFile tmp("paintnet_read_test.txt");
  std::ofstream(tmp.path, std::ios::binary) << "\xEF\xBB\xBF; comment\r\n\nFF123456\r\n80123456\r\n";
  auto pal = file::load_paintnet_file(tmp.path.c_str());
  ASSERT_NE(nullptr, pal);
  ASSERT_EQ(2, pal->size());
  EXPECT_EQ(rgba(0x12, 0x34, 0x56, 255), pal->entry(0));
  EXPECT_EQ(rgba(0x12, 0x34, 0x56, 0x80), pal->entry(1));
}

TEST(PaintNetFile, SaveLoadRoundTripKeepsArgbAlpha)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(0x12, 0x34, 0x56, 255));
  pal->setEntry(1, rgba(0xab, 0xcd, 0xef, 0x40));
  const TempFile tmp("paintnet_roundtrip.txt");

  ASSERT_TRUE(file::save_paintnet_file(*pal, tmp.path.c_str()));
  auto loaded = file::load_paintnet_file(tmp.path.c_str());
  ASSERT_NE(nullptr, loaded);
  ASSERT_EQ(2, loaded->size());
  EXPECT_EQ(pal->entry(0), loaded->entry(0));
  EXPECT_EQ(pal->entry(1), loaded->entry(1));
}

TEST(PaintNetFile, DoesNotTruncatePalettesAbovePaintNetRecommendation)
{
  auto pal = Palette::create(97);
  for (int i = 0; i < pal->size(); ++i)
    pal->setEntry(i, rgba(i, i, i, 255));
  const TempFile tmp("paintnet_large_test.txt");

  ASSERT_TRUE(file::save_paintnet_file(*pal, tmp.path.c_str()));
  auto loaded = file::load_paintnet_file(tmp.path.c_str());
  ASSERT_NE(nullptr, loaded);
  EXPECT_EQ(97, loaded->size());
}

TEST(PaintNetFile, RejectsMalformedEntries)
{
  const TempFile tmp("paintnet_invalid_test.txt");
  std::ofstream(tmp.path.c_str()) << "; header\n1234567\n";
  EXPECT_EQ(nullptr, file::load_paintnet_file(tmp.path.c_str()));
}

TEST(ActFile, ReadsColorCountAndTransparentEntry)
{
  const TempFile tmp("act_count_test.act");
  ASSERT_TRUE(write_bytes(tmp.path.c_str(), sample_act(2, 1)));

  auto pal = file::load_act_file(tmp.path.c_str());
  ASSERT_NE(nullptr, pal);
  ASSERT_EQ(2, pal->size());
  EXPECT_EQ(255, rgba_geta(pal->entry(0)));
  EXPECT_EQ(0, rgba_geta(pal->entry(1)));
  EXPECT_EQ(rgba(0x12, 0x34, 0x56, 255), pal->entry(0));
  EXPECT_EQ(rgba(0xab, 0xcd, 0xef, 0), pal->entry(1));
}

TEST(ActFile, ReadsLegacy256ColorFile)
{
  const TempFile tmp("act_legacy_test.act");
  std::vector<uint8_t> bytes(256 * 3, 0);
  ASSERT_TRUE(write_bytes(tmp.path.c_str(), bytes));
  auto pal = file::load_act_file(tmp.path.c_str());
  ASSERT_NE(nullptr, pal);
  EXPECT_EQ(256, pal->size());
  EXPECT_EQ(255, rgba_geta(pal->entry(0)));
}

TEST(ActFile, SaveLoadRoundTripStoresCountAndTransparency)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(0x12, 0x34, 0x56, 255));
  pal->setEntry(1, rgba(0xab, 0xcd, 0xef, 0));
  const TempFile tmp("act_roundtrip_test.act");

  ASSERT_TRUE(file::save_act_file(*pal, tmp.path.c_str()));
  std::ifstream in(tmp.path, std::ios::binary);
  const std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  ASSERT_EQ(256 * 3 + 4, bytes.size());
  EXPECT_EQ(0, bytes[768]);
  EXPECT_EQ(2, bytes[769]);
  EXPECT_EQ(0, bytes[770]);
  EXPECT_EQ(1, bytes[771]);

  auto loaded = file::load_act_file(tmp.path.c_str());
  ASSERT_NE(nullptr, loaded);
  ASSERT_EQ(2, loaded->size());
  EXPECT_EQ(rgba(0x12, 0x34, 0x56, 255), loaded->entry(0));
  EXPECT_EQ(rgba(0xab, 0xcd, 0xef, 0), loaded->entry(1));
}

TEST(ActFile, RejectsTruncatedAndInvalidMetadata)
{
  const TempFile tmp("act_invalid_test.act");
  ASSERT_TRUE(write_bytes(tmp.path.c_str(), std::vector<uint8_t>(767, 0)));
  EXPECT_EQ(nullptr, file::load_act_file(tmp.path.c_str()));

  ASSERT_TRUE(write_bytes(tmp.path.c_str(), sample_act(257, 0xffff)));
  EXPECT_EQ(nullptr, file::load_act_file(tmp.path.c_str()));

  ASSERT_TRUE(write_bytes(tmp.path.c_str(), sample_act(2, 2)));
  EXPECT_EQ(nullptr, file::load_act_file(tmp.path.c_str()));
}

TEST(ActFile, RejectsPartialAlphaInsteadOfSilentlyDroppingIt)
{
  auto pal = Palette::create(1);
  pal->setEntry(0, rgba(1, 2, 3, 128));
  const TempFile tmp("act_partial_alpha_test.act");
  EXPECT_FALSE(file::save_act_file(*pal, tmp.path.c_str()));
}

TEST(ActFile, RejectsMultipleTransparentEntries)
{
  auto pal = Palette::create(2);
  pal->setEntry(0, rgba(1, 2, 3, 0));
  pal->setEntry(1, rgba(4, 5, 6, 0));
  const TempFile tmp("act_multiple_transparent_test.act");
  EXPECT_FALSE(file::save_act_file(*pal, tmp.path.c_str()));
}

TEST(HexFile, RejectsEmptyPaletteOnSave)
{
  const auto pal = Palette::create(0);
  const TempFile tmp("hex_empty_test.hex");
  EXPECT_FALSE(file::save_hex_file(*pal, tmp.path.c_str()));
  EXPECT_FALSE(std::filesystem::exists(tmp.path));
}

TEST(PaintNetFile, RejectsEmptyPaletteOnSave)
{
  const auto pal = Palette::create(0);
  const TempFile tmp("paintnet_empty_test.txt");
  EXPECT_FALSE(file::save_paintnet_file(*pal, tmp.path.c_str()));
  EXPECT_FALSE(std::filesystem::exists(tmp.path));
}
