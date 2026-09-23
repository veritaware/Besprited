// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "base/sha1_rfc3174.h"
#include "doc/image.h"
#include "doc/palette.h"
#include "doc/primitives.h"
#include "doc/rgbmap.h"
#include "filters/color_curve.h"
#include "filters/color_curve_filter.h"
#include "filters/convolution_matrix.h"
#include "filters/convolution_matrix_filter.h"
#include "filters/filter_indexed_data.h"
#include "filters/filter_manager.h"
#include "filters/invert_color_filter.h"
#include "filters/median_filter.h"
#include "filters/replace_color_filter.h"
#include "filters/target.h"

#include <memory>
#include <string>

using namespace doc;
using namespace filters;

// Golden-hash safety net for issue #226: before touching any filter code to
// collapse the 5 filters' duplicated apply-loop skeletons, this records a
// SHA-1 of each filter's pixel output against a seeded synthetic image, for
// every pixel format and a couple of Target masks. Any future refactor of
// filters/*.cpp must keep every one of these hashes byte-for-byte identical.
//
// Also includes a few cheap property tests (invert-twice==identity, etc.)
// per the issue's suggestion - these make a regression failure readable
// ("doubly-inverted pixel changed") instead of just "hash differs".

namespace
{

// TestFilterManager is a minimal FilterManager - it drives a Filter over
// every row of a single in-memory Image with no mask (skipPixel() is always
// false) and no progress/cancellation machinery, unlike
// app::FilterManagerImpl (which needs a real Context/Document/Transaction).
// See the FilterManager interface comment in filters/filter_manager.h for
// the exact per-call contract this implements.
class TestFilterManager : public FilterManager, public FilterIndexedData
{
public:
  TestFilterManager(Image* src, Image* dst, Target target,
                    Palette* pal = nullptr, RgbMap* rgbMap = nullptr)
    : m_src(src)
    , m_dst(dst)
    , m_target(target)
    , m_pal(pal)
    , m_rgbMap(rgbMap)
  {
  }

  void run(Filter& filter)
  {
    for (m_row = 0; m_row < m_src->height(); ++m_row)
    {
      switch (m_src->pixelFormat())
      {
      case IMAGE_RGB:
        filter.applyToRgba(this);
        break;
      case IMAGE_GRAYSCALE:
        filter.applyToGrayscale(this);
        break;
      case IMAGE_INDEXED:
        filter.applyToIndexed(this);
        break;
      default:
        FAIL() << "TestFilterManager only supports RGB/Grayscale/Indexed";
        break;
      }
    }
  }

  // FilterManager impl
  const void* getSourceAddress() override
  {
    return m_src->getPixelAddress(0, m_row);
  }
  void* getDestinationAddress() override
  {
    return m_dst->getPixelAddress(0, m_row);
  }
  int getWidth() override { return m_src->width(); }
  Target getTarget() override { return m_target; }
  FilterIndexedData* getIndexedData() override { return this; }
  bool skipPixel() override { return false; }
  const Image* getSourceImage() override { return m_src; }
  int x() override { return 0; }
  int y() override { return m_row; }

  // FilterIndexedData impl
  Palette* getPalette() override { return m_pal; }
  RgbMap* getRgbMap() override { return m_rgbMap; }

private:
  Image* m_src;
  Image* m_dst;
  Target m_target;
  Palette* m_pal;
  RgbMap* m_rgbMap;
  int m_row = 0;
};

// A deterministic, non-uniform pattern - enough variation that a filter bug
// touching only some channels/pixels would still change the hash.
std::unique_ptr<Image> makeSeededImage(PixelFormat format, int w, int h)
{
  std::unique_ptr<Image> img(Image::create(format, w, h));
  for (int y = 0; y < h; ++y)
  {
    for (int x = 0; x < w; ++x)
    {
      switch (format)
      {
      case IMAGE_RGB:
        put_pixel(img.get(), x, y,
                  rgba((x * 37 + y * 11) & 0xff, (x * 13 + y * 29) & 0xff,
                       (x * 7 + y * 53) & 0xff, ((x + y) % 2) ? 255 : 200));
        break;
      case IMAGE_GRAYSCALE:
        put_pixel(img.get(), x, y,
                  graya((x * 23 + y * 17) & 0xff,
                        ((x + y) % 3 == 0) ? 255 : 180));
        break;
      case IMAGE_INDEXED:
        put_pixel(img.get(), x, y, (x * 3 + y * 5) & 0xff);
        break;
      default:
        break;
      }
    }
  }
  return img;
}

std::shared_ptr<Palette> makeTestPalette()
{
  auto pal = Palette::create(256);
  for (int i = 0; i < 256; ++i)
    pal->setEntry(i, rgba(i, (i * 3) & 0xff, (255 - i) & 0xff, 255));
  return pal;
}

std::shared_ptr<RgbMap> makeRgbMap(const Palette* pal)
{
  auto map = std::make_shared<RgbMap>();
  map->regenerate(pal, -1);
  return map;
}

std::string sha1Hex(const void* data, std::size_t size)
{
  SHA1Context ctx;
  SHA1Reset(&ctx);
  SHA1Input(&ctx, static_cast<const uint8_t*>(data),
           static_cast<unsigned int>(size));
  uint8_t digest[SHA1HashSize];
  SHA1Result(&ctx, digest);

  static const char* hexDigits = "0123456789abcdef";
  std::string hex;
  hex.reserve(SHA1HashSize * 2);
  for (uint8_t byte : digest)
  {
    hex.push_back(hexDigits[byte >> 4]);
    hex.push_back(hexDigits[byte & 0xf]);
  }
  return hex;
}

// Hashes the raw pixel buffer - getPixelAddress(0, 0) plus the full
// row-stride-based byte size covers every pixel regardless of format.
std::string hashImage(const Image* img)
{
  std::size_t size =
      static_cast<std::size_t>(img->getRowStrideSize()) * img->height();
  return sha1Hex(img->getPixelAddress(0, 0), size);
}

constexpr int kW = 13;
constexpr int kH = 9;

} // namespace

// --- Golden hashes: InvertColorFilter -------------------------------------

TEST(FilterGolden, InvertRgbAllChannels)
{
  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto dst = makeSeededImage(IMAGE_RGB, kW, kH);
  InvertColorFilter filter;
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);
  EXPECT_EQ("6eacd7361bc37bba87a7418f706b935114806aca", hashImage(dst.get()));
}

TEST(FilterGolden, InvertRgbRedOnly)
{
  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto dst = makeSeededImage(IMAGE_RGB, kW, kH);
  InvertColorFilter filter;
  TestFilterManager mgr(src.get(), dst.get(), TARGET_RED_CHANNEL);
  mgr.run(filter);
  EXPECT_EQ("94079e56c3d3c9e3f55c39524a25f2c93dbf1274", hashImage(dst.get()));
}

TEST(FilterGolden, InvertGrayscale)
{
  auto src = makeSeededImage(IMAGE_GRAYSCALE, kW, kH);
  auto dst = makeSeededImage(IMAGE_GRAYSCALE, kW, kH);
  InvertColorFilter filter;
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);
  EXPECT_EQ("20c143b9256735ccad9fe7101c2c0492f5424d42", hashImage(dst.get()));
}

TEST(FilterGolden, InvertIndexedIndexChannel)
{
  auto src = makeSeededImage(IMAGE_INDEXED, kW, kH);
  auto dst = makeSeededImage(IMAGE_INDEXED, kW, kH);
  InvertColorFilter filter;
  TestFilterManager mgr(src.get(), dst.get(), TARGET_INDEX_CHANNEL);
  mgr.run(filter);
  EXPECT_EQ("ed2365fdba273d7fef823cbb64ee11b7228ffaa5", hashImage(dst.get()));
}

TEST(FilterGolden, InvertIndexedRgbChannels)
{
  auto pal = makeTestPalette();
  auto rgbMap = makeRgbMap(pal.get());
  auto src = makeSeededImage(IMAGE_INDEXED, kW, kH);
  auto dst = makeSeededImage(IMAGE_INDEXED, kW, kH);
  InvertColorFilter filter;
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS, pal.get(),
                        rgbMap.get());
  mgr.run(filter);
  EXPECT_EQ("826f3ba912ce0589d6404966a0bc43ab9f611c01", hashImage(dst.get()));
}

// --- Golden hashes: ReplaceColorFilter -------------------------------------

TEST(FilterGolden, ReplaceColorRgb)
{
  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto dst = makeSeededImage(IMAGE_RGB, kW, kH);
  ReplaceColorFilter filter;
  filter.setFrom(rgba(50, 50, 50, 255));
  filter.setTo(rgba(0, 255, 0, 255));
  filter.setTolerance(80);
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);
  EXPECT_EQ("924ccd58c7459bf8ca305a562f58ccff02901abb", hashImage(dst.get()));
}

TEST(FilterGolden, ReplaceColorIndexed)
{
  auto pal = makeTestPalette();
  auto rgbMap = makeRgbMap(pal.get());
  auto src = makeSeededImage(IMAGE_INDEXED, kW, kH);
  auto dst = makeSeededImage(IMAGE_INDEXED, kW, kH);
  ReplaceColorFilter filter;
  filter.setFrom(10);
  filter.setTo(200);
  filter.setTolerance(5);
  TestFilterManager mgr(src.get(), dst.get(), TARGET_INDEX_CHANNEL, pal.get(),
                        rgbMap.get());
  mgr.run(filter);
  EXPECT_EQ("e7588511aee8f80b56aa37fce50c319dc8c4ee7f", hashImage(dst.get()));
}

// --- Golden hashes: ColorCurveFilter ---------------------------------------

TEST(FilterGolden, ColorCurveRgb)
{
  ColorCurve curve(ColorCurve::Linear);
  curve.addPoint(gfx::Point(0, 0));
  curve.addPoint(gfx::Point(128, 200));
  curve.addPoint(gfx::Point(255, 255));

  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto dst = makeSeededImage(IMAGE_RGB, kW, kH);
  ColorCurveFilter filter;
  filter.setCurve(&curve);
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);
  EXPECT_EQ("73eeddfedaa06223c4d77d17e0953dc880e45feb", hashImage(dst.get()));
}

// --- Property tests (self-verifying, no magic hashes) ----------------------

TEST(FilterProperty, InvertTwiceIsIdentity)
{
  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto once = makeSeededImage(IMAGE_RGB, kW, kH);
  auto twice = makeSeededImage(IMAGE_RGB, kW, kH);

  InvertColorFilter filter;
  TestFilterManager mgr1(src.get(), once.get(), TARGET_ALL_CHANNELS);
  mgr1.run(filter);
  TestFilterManager mgr2(once.get(), twice.get(), TARGET_ALL_CHANNELS);
  mgr2.run(filter);

  EXPECT_EQ(hashImage(src.get()), hashImage(twice.get()));
}

TEST(FilterProperty, ReplaceColorWithZeroToleranceAndSameFromToIsIdentity)
{
  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto dst = makeSeededImage(IMAGE_RGB, kW, kH);

  ReplaceColorFilter filter;
  filter.setFrom(rgba(1, 2, 3, 4));
  filter.setTo(rgba(1, 2, 3, 4));
  filter.setTolerance(0);
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);

  EXPECT_EQ(hashImage(src.get()), hashImage(dst.get()));
}

TEST(FilterProperty, MedianOnAConstantImageIsIdentity)
{
  auto src = std::unique_ptr<Image>(Image::create(IMAGE_RGB, kW, kH));
  clear_image(src.get(), rgba(30, 60, 90, 255));
  auto dst = std::unique_ptr<Image>(Image::create(IMAGE_RGB, kW, kH));
  clear_image(dst.get(), 0);

  MedianFilter filter;
  filter.setTiledMode(TiledMode::NONE);
  filter.setSize(3, 3);
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);

  EXPECT_EQ(hashImage(src.get()), hashImage(dst.get()));
}

TEST(FilterProperty, ConvolutionMatrixIdentityKernelIsIdentity)
{
  auto matrix = base::SharedPtr<ConvolutionMatrix>(new ConvolutionMatrix(3, 3));
  for (int y = 0; y < 3; ++y)
    for (int x = 0; x < 3; ++x)
      matrix->value(x, y) = 0;
  matrix->value(1, 1) = 1;
  matrix->setCenterX(1);
  matrix->setCenterY(1);
  matrix->setDiv(1);
  matrix->setBias(0);
  matrix->setDefaultTarget(TARGET_ALL_CHANNELS);

  auto src = makeSeededImage(IMAGE_RGB, kW, kH);
  auto dst = makeSeededImage(IMAGE_RGB, kW, kH);

  ConvolutionMatrixFilter filter;
  filter.setMatrix(matrix);
  filter.setTiledMode(TiledMode::NONE);
  TestFilterManager mgr(src.get(), dst.get(), TARGET_ALL_CHANNELS);
  mgr.run(filter);

  EXPECT_EQ(hashImage(src.get()), hashImage(dst.get()));
}
