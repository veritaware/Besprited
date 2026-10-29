// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"
#include "app/file/ase_reader.h"

#include <cstdio>
#include <memory>

namespace
{

class AseReaderTest : public ::testing::Test
{
protected:
  std::unique_ptr<FILE, decltype(&fclose)> file{nullptr, &fclose};

  void SetUp() override
  {
    file.reset(std::tmpfile());
    ASSERT_NE(nullptr, file);
    const uint8_t bytes[] = {0x34, 0x12, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 'x'};
    ASSERT_EQ(sizeof(bytes), fwrite(bytes, 1, sizeof(bytes), file.get()));
    ASSERT_EQ(0, fseek(file.get(), 0, SEEK_SET));
  }
};

TEST_F(AseReaderTest, ReadsUnsignedLittleEndianValuesAndStrings)
{
  app::AseReader reader(file.get(), 9);
  EXPECT_EQ(0x1234u, reader.word());
  EXPECT_EQ(0xffffffffu, reader.dword());
  EXPECT_EQ("x", reader.string());
  EXPECT_EQ(0u, reader.remaining());
}

TEST_F(AseReaderTest, RejectsCrossingBoundaryBeforeConsumingFollowingBytes)
{
  app::AseReader reader(file.get(), 1);
  EXPECT_THROW(reader.word(), base::Exception);
  EXPECT_EQ(0, ftell(file.get()));
  EXPECT_EQ(0x34, reader.byte());
  EXPECT_THROW(reader.byte(), base::Exception);
  EXPECT_EQ(1, ftell(file.get()));
  EXPECT_EQ(0x12, fgetc(file.get()));
}

TEST_F(AseReaderTest, RejectsIncompletePhysicalRead)
{
  ASSERT_EQ(0, fseek(file.get(), 8, SEEK_SET));
  app::AseReader reader(file.get(), 4);
  EXPECT_THROW(reader.dword(), base::Exception);
  EXPECT_TRUE(feof(file.get()));
}

TEST_F(AseReaderTest, RejectsStringLengthBeforeReadingItsContents)
{
  ASSERT_EQ(0, fseek(file.get(), 6, SEEK_SET));
  app::AseReader reader(file.get(), 2);
  EXPECT_THROW(reader.string(), base::Exception);
  EXPECT_EQ(8, ftell(file.get()));
  EXPECT_EQ('x', fgetc(file.get()));
}

TEST_F(AseReaderTest, SkipChecksBoundsAndPhysicalEof)
{
  app::AseReader reader(file.get(), 10);
  EXPECT_THROW(reader.skip(11), base::Exception);
  EXPECT_EQ(0, ftell(file.get()));
  reader.skip(8);
  EXPECT_EQ(2u, reader.remaining());
  EXPECT_THROW(reader.skip(2), base::Exception);
}

} // namespace
