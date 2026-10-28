// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/transformation.h"
#include "base/pi.h"
#include "doc/algorithm/rotate.h"
#include "doc/algorithm/shrink_bounds.h"
#include "doc/image.h"
#include "doc/primitives.h"

#include <memory>

using namespace app;
using namespace doc;

namespace
{

Transformation makeTransformation(const gfx::RectF& bounds, double angle)
{
  Transformation t(bounds);
  t.angle(angle);
  return t;
}

// Every pixel gets a distinct opaque color so misplaced pixels are detected.
std::unique_ptr<Image> makeNumberedImage(int w, int h)
{
  std::unique_ptr<Image> img(Image::create(IMAGE_RGB, w, h));
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
      put_pixel(img.get(), x, y, rgba(x + 1, y + 1, 0, 255));
  return img;
}

// Rotates the image by quarterTurns * 90 degrees counter-clockwise (on screen).
std::unique_ptr<Image> rotateQuarterTurns(const Image* src, int quarterTurns)
{
  const int k = ((quarterTurns % 4) + 4) % 4;
  const int w = src->width(), h = src->height();
  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, k % 2 ? h : w, k % 2 ? w : h));
  for (int y = 0; y < h; ++y)
    for (int x = 0; x < w; ++x)
    {
      int u = x, v = y;
      switch (k)
      {
      case 1: u = y; v = w - 1 - x; break;
      case 2: u = w - 1 - x; v = h - 1 - y; break;
      case 3: u = h - 1 - y; v = x; break;
      }
      put_pixel(dst.get(), u, v, get_pixel(src, x, y));
    }
  return dst;
}

// Same steps as PixelsMovement::drawParallelogram() with the FAST algorithm.
std::unique_ptr<Image> applyFastRotation(const Image* src, const Transformation& t)
{
  Transformation::Corners corners;
  t.transformBox(corners);
  const gfx::Rect bounds = corners.bounds();
  const gfx::Point lt = bounds.origin();

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, bounds.w, bounds.h));
  clear_image(dst.get(), 0);
  algorithm::parallelogram(dst.get(), src, nullptr, int(corners.leftTop().x - lt.x), int(corners.leftTop().y - lt.y),
                           int(corners.rightTop().x - lt.x), int(corners.rightTop().y - lt.y),
                           int(corners.rightBottom().x - lt.x), int(corners.rightBottom().y - lt.y),
                           int(corners.leftBottom().x - lt.x), int(corners.leftBottom().y - lt.y));
  return dst;
}

} // namespace

TEST(Transformation, QuarterTurnsAreDetected)
{
  const gfx::RectF bounds(10, 20, 8, 5);
  const gfx::Size size(8, 5);

  for (int k = -8; k <= 8; ++k)
    EXPECT_TRUE(makeTransformation(bounds, k * PI / 2.0).isUnscaledQuarterTurn(size)) << "k=" << k;

  // As produced by the editor's degrees -> radians snapping.
  EXPECT_TRUE(makeTransformation(bounds, PI * 90.0 / 180.0).isUnscaledQuarterTurn(size));
  EXPECT_TRUE(makeTransformation(bounds, PI * -270.0 / 180.0).isUnscaledQuarterTurn(size));
}

TEST(Transformation, OtherAnglesAreRejected)
{
  const gfx::RectF bounds(0, 0, 8, 5);
  const gfx::Size size(8, 5);

  for (double deg : {1.0, 30.0, 45.0, 89.0, 90.1, 135.0, 179.5, -45.0, -91.0})
    EXPECT_FALSE(makeTransformation(bounds, PI * deg / 180.0).isUnscaledQuarterTurn(size)) << "deg=" << deg;
}

TEST(Transformation, ScaledQuarterTurnsAreRejected)
{
  const gfx::Size size(8, 5);

  EXPECT_FALSE(makeTransformation(gfx::RectF(0, 0, 16, 5), PI / 2.0).isUnscaledQuarterTurn(size));
  EXPECT_FALSE(makeTransformation(gfx::RectF(0, 0, 8, 10), 0.0).isUnscaledQuarterTurn(size));
  // Bounds are pre-rotation, so swapped dimensions are a scale, not a turn.
  EXPECT_FALSE(makeTransformation(gfx::RectF(0, 0, 5, 8), PI / 2.0).isUnscaledQuarterTurn(size));
}

TEST(Transformation, FastAlgorithmIsPixelPerfectOnQuarterTurns)
{
  // Even/odd size combinations exercise integer and half-pixel pivots.
  for (const gfx::Size size : {gfx::Size(4, 4), gfx::Size(5, 5), gfx::Size(6, 4), gfx::Size(7, 3)})
  {
    const auto src = makeNumberedImage(size.w, size.h);

    for (int k = -4; k <= 4; ++k)
    {
      SCOPED_TRACE(testing::Message() << size.w << "x" << size.h << " k=" << k);

      const Transformation t = makeTransformation(gfx::RectF(3, 2, size.w, size.h), k * PI / 2.0);
      ASSERT_TRUE(t.isUnscaledQuarterTurn(src->size()));

      const auto dst = applyFastRotation(src.get(), t);
      gfx::Rect content;
      ASSERT_TRUE(algorithm::shrink_bounds(dst.get(), content, 0));

      const auto expected = rotateQuarterTurns(src.get(), k);
      ASSERT_EQ(expected->size(), content.size());
      for (int y = 0; y < content.h; ++y)
        for (int x = 0; x < content.w; ++x)
          ASSERT_EQ(get_pixel(expected.get(), x, y), get_pixel(dst.get(), content.x + x, content.y + y))
            << "at " << x << "," << y;
    }
  }
}
