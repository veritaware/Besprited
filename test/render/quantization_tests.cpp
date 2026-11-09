// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tests.h"

#include "doc/color.h"
#include "doc/image.h"
#include "doc/image_ref.h"
#include "doc/palette.h"
#include "doc/primitives.h"
#include "render/quantization.h"

using namespace doc;
using namespace render;

namespace
{

color_t rgbaToGray(const color_t c, const GrayscaleMethod method = GrayscaleMethod::LUMA_709)
{
  ImageRef src(Image::create(IMAGE_RGB, 1, 1));
  put_pixel(src.get(), 0, 0, c);
  ImageRef dst(
    convert_pixel_format(src.get(), nullptr, IMAGE_GRAYSCALE, DitheringMethod::NONE, nullptr, nullptr, false, 0, method));
  return get_pixel(dst.get(), 0, 0);
}

color_t indexToGray(const Palette& palette, const color_t index, const bool isBackground, const color_t maskColor,
                    const GrayscaleMethod method = GrayscaleMethod::LUMA_709)
{
  ImageRef src(Image::create(IMAGE_INDEXED, 1, 1));
  src->setMaskColor(maskColor);
  put_pixel(src.get(), 0, 0, index);
  ImageRef dst(convert_pixel_format(src.get(), nullptr, IMAGE_GRAYSCALE, DitheringMethod::NONE, nullptr, &palette,
                                    isBackground, maskColor, method));
  return get_pixel(dst.get(), 0, 0);
}

} // namespace

TEST(Quantization, RgbToGrayscaleUsesRec709)
{
  EXPECT_EQ(54, graya_getv(rgbaToGray(rgba(255, 0, 0, 255))));
  EXPECT_EQ(182, graya_getv(rgbaToGray(rgba(0, 255, 0, 255))));
  EXPECT_EQ(18, graya_getv(rgbaToGray(rgba(0, 0, 255, 255))));
}

TEST(Quantization, RgbToGrayscaleKeepsGraysAndAlpha)
{
  for (const int v : {0, 1, 127, 128, 254, 255})
  {
    const color_t g = rgbaToGray(rgba(v, v, v, 200));
    EXPECT_EQ(v, graya_getv(g)) << v;
    EXPECT_EQ(200, graya_geta(g)) << v;
  }
}

TEST(Quantization, IndexedToGrayscaleUsesPaletteEntry)
{
  const auto palettePtr = Palette::create(2);
  Palette& palette = *palettePtr;
  palette.setEntry(0, rgba(0, 0, 0, 0));
  palette.setEntry(1, rgba(255, 0, 0, 255));

  const color_t g = indexToGray(palette, 1, false, 0);
  EXPECT_EQ(54, graya_getv(g));
  EXPECT_EQ(255, graya_geta(g));
}

TEST(Quantization, IndexedToGrayscaleMaskColor)
{
  const auto palettePtr = Palette::create(2);
  Palette& palette = *palettePtr;
  palette.setEntry(0, rgba(128, 128, 128, 255));
  palette.setEntry(1, rgba(255, 0, 0, 255));

  // Mask index on a transparent layer becomes fully transparent.
  EXPECT_EQ(graya(0, 0), indexToGray(palette, 0, false, 0));

  // On a background layer it is a regular palette entry.
  const color_t g = indexToGray(palette, 0, true, 0);
  EXPECT_EQ(128, graya_getv(g));
  EXPECT_EQ(255, graya_geta(g));
}

TEST(Quantization, RgbToGrayscaleHsvValueUsesMaxChannel)
{
  const color_t g = rgbaToGray(rgba(10, 200, 30, 180), GrayscaleMethod::HSV_VALUE);
  EXPECT_EQ(200, graya_getv(g));
  EXPECT_EQ(180, graya_geta(g));
  EXPECT_EQ(255, graya_getv(rgbaToGray(rgba(255, 0, 0, 255), GrayscaleMethod::HSV_VALUE)));
}

TEST(Quantization, RgbToGrayscaleHsvValueKeepsGrays)
{
  for (const int v : {0, 1, 127, 128, 254, 255})
    EXPECT_EQ(v, graya_getv(rgbaToGray(rgba(v, v, v, 255), GrayscaleMethod::HSV_VALUE))) << v;
}

TEST(Quantization, IndexedToGrayscaleHsvValue)
{
  const auto palettePtr = Palette::create(2);
  palettePtr->setEntry(0, rgba(0, 0, 0, 0));
  palettePtr->setEntry(1, rgba(0, 0, 255, 255));

  EXPECT_EQ(255, graya_getv(indexToGray(*palettePtr, 1, false, 0, GrayscaleMethod::HSV_VALUE)));
}

TEST(Quantization, RgbToGray)
{
  EXPECT_EQ(54, rgb_to_gray(255, 0, 0, GrayscaleMethod::LUMA_709));
  EXPECT_EQ(255, rgb_to_gray(255, 0, 0, GrayscaleMethod::HSV_VALUE));
  EXPECT_EQ(128, rgb_to_gray(128, 128, 128, GrayscaleMethod::LUMA_709));
  EXPECT_EQ(128, rgb_to_gray(128, 128, 128, GrayscaleMethod::HSV_VALUE));
}

TEST(Quantization, PaletteOptimizerReservesTransparentSlot)
{
  ImageRef img(Image::create(IMAGE_RGB, 3, 1));
  put_pixel(img.get(), 0, 0, rgba(10, 20, 30, 255)); // First pixel is solid
  put_pixel(img.get(), 1, 0, rgba(200, 100, 50, 255));
  put_pixel(img.get(), 2, 0, rgba(0, 0, 0, 0));

  PaletteOptimizer optimizer;
  optimizer.feedWithImage(img.get(), true);

  const auto palette = Palette::create(256);
  optimizer.calculate(*palette, 0, nullptr);

  ASSERT_EQ(3, palette->size());
  EXPECT_EQ(rgba(0, 0, 0, 0), palette->getEntry(0));
  EXPECT_NE(palette->getEntry(1), palette->getEntry(2));
  for (const color_t expected : {rgba(10, 20, 30, 255), rgba(200, 100, 50, 255)})
    EXPECT_TRUE(palette->getEntry(1) == expected || palette->getEntry(2) == expected);
}

TEST(Quantization, PaletteOptimizerWithoutMaskIndexKeepsFirstSlotFree)
{
  ImageRef img(Image::create(IMAGE_RGB, 1, 1));
  put_pixel(img.get(), 0, 0, rgba(10, 20, 30, 255));

  PaletteOptimizer optimizer;
  optimizer.feedWithImage(img.get(), true);

  const auto palette = Palette::create(256);
  optimizer.calculate(*palette, -1, nullptr);

  ASSERT_EQ(1, palette->size());
  EXPECT_EQ(rgba(10, 20, 30, 255), palette->getEntry(0));
}
