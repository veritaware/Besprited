// Render Tests
// Aseprite  | Copyright (C) 2001-2014 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "tests.h"

#include "render/render.h"

#include "doc/cel.h"
#include "doc/context.h"
#include "doc/document.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/palette.h"
#include "doc/primitives.h"

using namespace doc;
using namespace render;

template <typename T> class RenderAllModes : public testing::Test
{
protected:
  RenderAllModes() = default;
};

using ImageAllTraits =
    testing::Types<RgbTraits, GrayscaleTraits, IndexedTraits>;
TYPED_TEST_CASE(RenderAllModes, ImageAllTraits);

// a b
// c d
#define EXPECT_2X2_PIXELS(image, a, b, c, d)                                   \
  EXPECT_EQ(a, get_pixel(image, 0, 0));                                        \
  EXPECT_EQ(b, get_pixel(image, 1, 0));                                        \
  EXPECT_EQ(c, get_pixel(image, 0, 1));                                        \
  EXPECT_EQ(d, get_pixel(image, 1, 1))

// a b c d
// e f g h
// i j k l
// m n o p
#define EXPECT_4X4_PIXELS(image, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o,  \
                          p)                                                   \
  EXPECT_EQ(a, get_pixel(image, 0, 0));                                        \
  EXPECT_EQ(b, get_pixel(image, 1, 0));                                        \
  EXPECT_EQ(c, get_pixel(image, 2, 0));                                        \
  EXPECT_EQ(d, get_pixel(image, 3, 0));                                        \
  EXPECT_EQ(e, get_pixel(image, 0, 1));                                        \
  EXPECT_EQ(f, get_pixel(image, 1, 1));                                        \
  EXPECT_EQ(g, get_pixel(image, 2, 1));                                        \
  EXPECT_EQ(h, get_pixel(image, 3, 1));                                        \
  EXPECT_EQ(i, get_pixel(image, 0, 2));                                        \
  EXPECT_EQ(j, get_pixel(image, 1, 2));                                        \
  EXPECT_EQ(k, get_pixel(image, 2, 2));                                        \
  EXPECT_EQ(l, get_pixel(image, 3, 2));                                        \
  EXPECT_EQ(m, get_pixel(image, 0, 3));                                        \
  EXPECT_EQ(n, get_pixel(image, 1, 3));                                        \
  EXPECT_EQ(o, get_pixel(image, 2, 3));                                        \
  EXPECT_EQ(p, get_pixel(image, 3, 3))

TEST(Render, Basic)
{
  Context ctx;
  Document* doc = ctx.documents().add(2, 2, ColorMode::RGB);

  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 2);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 2, 2));
  clear_image(dst.get(), 1);
  EXPECT_2X2_PIXELS(dst.get(), 1, 1, 1, 1);

  Render render;
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  EXPECT_2X2_PIXELS(dst.get(), 2, 2, 2, 2);
}

TYPED_TEST(RenderAllModes, CheckDefaultBackgroundMode)
{
  using ImageTraits = TypeParam;

  Context ctx;
  Document* doc =
      ctx.documents().add(2, 2, ColorMode(ImageTraits::pixel_format));

  EXPECT_TRUE(!doc->sprite()->layer(0)->isBackground());
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 0);
  put_pixel(src, 1, 1, 1);

  std::unique_ptr<Image> dst(Image::create(ImageTraits::pixel_format, 2, 2));
  clear_image(dst.get(), 1);
  EXPECT_2X2_PIXELS(dst.get(), 1, 1, 1, 1);

  Render render;
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  // Default background mode is to set all pixels to transparent color
  EXPECT_2X2_PIXELS(dst.get(), 0, 0, 0, 1);
}

TEST(Render, DefaultBackgroundModeWithNonzeroTransparentIndex)
{
  Context ctx;
  Document* doc = ctx.documents().add(2, 2, ColorMode::INDEXED);
  doc->sprite()->setTransparentColor(2); // Transparent color is index 2

  EXPECT_TRUE(!doc->sprite()->layer(0)->isBackground());
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 2);
  put_pixel(src, 1, 1, 1);

  std::unique_ptr<Image> dst(Image::create(IMAGE_INDEXED, 2, 2));
  clear_image(dst.get(), 1);
  EXPECT_2X2_PIXELS(dst.get(), 1, 1, 1, 1);

  Render render;
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  EXPECT_2X2_PIXELS(dst.get(), 2, 2, 2, 1); // Indexed transparent

  dst.reset(Image::create(IMAGE_RGB, 2, 2));
  clear_image(dst.get(), 1);
  EXPECT_2X2_PIXELS(dst.get(), 1, 1, 1, 1);
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  color_t c1 = doc->sprite()->palette(0)->entry(1);
  EXPECT_NE(0, c1);
  EXPECT_2X2_PIXELS(dst.get(), 0, 0, 0, c1); // RGB transparent
}

TEST(Render, CheckedBackground)
{
  Context ctx;
  Document* doc = ctx.documents().add(4, 4, ColorMode::RGB);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 4, 4));
  clear_image(dst.get(), 0);

  Render render;
  render.setBgType(BgType::CHECKED);
  render.setBgZoom(true);
  render.setBgColor1(1);
  render.setBgColor2(2);

  render.setBgCheckedSize(gfx::Size(1, 1));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst.get(), 1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1);

  render.setBgCheckedSize(gfx::Size(2, 2));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1);

  render.setBgCheckedSize(gfx::Size(3, 3));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1);

  render.setBgCheckedSize(gfx::Size(2, 3));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1);

  render.setBgCheckedSize(gfx::Size(1, 1));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(dst->bounds()), Zoom(2, 1));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 2, 2, 1, 1, 2, 2, 2, 2, 1, 1, 2, 2, 1, 1);

  // With bgZoom on, a checker tile scales together with the zoom factor:
  // size(1,1) at 3x zoom covers the same ground as size(3,3) at 1x zoom.
  render.setBgCheckedSize(gfx::Size(1, 1));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(dst->bounds()), Zoom(3, 1));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1);

  // With bgZoom off, the tile size is left alone regardless of zoom (as
  // long as it isn't smaller than one zoomed pixel, which would otherwise
  // clamp it back up) - so this is the same span-3-columns pattern as
  // size(3,3) at 1x zoom above, not the fully-covered single tile you'd
  // get if the size were scaled by 2x too.
  render.setBgZoom(false);
  render.setBgCheckedSize(gfx::Size(3, 3));
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(dst->bounds()), Zoom(2, 1));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 2, 2, 2, 1);

  // ... while with bgZoom back on, that same size(3,3) tile at 2x zoom
  // becomes a single 6x6 tile - larger than the whole 4x4 canvas, so it's
  // one solid color.
  render.setBgZoom(true);
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(dst->bounds()), Zoom(2, 1));
  EXPECT_4X4_PIXELS(dst.get(), 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
}

TEST(Render, CheckedBackgroundWithAnOddViewportOffset)
{
  Context ctx;
  Document* doc = ctx.documents().add(4, 4, ColorMode::RGB);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 4, 4));

  Render render;
  render.setBgType(BgType::CHECKED);
  render.setBgZoom(true);
  render.setBgColor1(1);
  render.setBgColor2(2);

  // A source-space offset of one tile shifts the checker phase by one
  // step, inverting the pattern relative to an unscrolled view - whether
  // the offset is horizontal or vertical.
  render.setBgCheckedSize(gfx::Size(1, 1));
  clear_image(dst.get(), 0);
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(0, 0, 1, 0, 4, 4));
  EXPECT_4X4_PIXELS(dst.get(), 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2);

  clear_image(dst.get(), 0);
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(0, 0, 0, 1, 4, 4));
  EXPECT_4X4_PIXELS(dst.get(), 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2);

  // Offsetting by one tile in *both* directions flips the parity twice,
  // landing back on the unscrolled pattern.
  clear_image(dst.get(), 0);
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(0, 0, 1, 1, 4, 4));
  EXPECT_4X4_PIXELS(dst.get(), 1, 2, 1, 2, 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1);

  // An offset that isn't a multiple of a (now 2px-wide) tile shifts each
  // tile's visible split point rather than just flipping the parity.
  render.setBgCheckedSize(gfx::Size(2, 2));
  clear_image(dst.get(), 0);
  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(0, 0, 1, 0, 4, 4));
  EXPECT_4X4_PIXELS(dst.get(), 1, 2, 2, 1, 1, 2, 2, 1, 2, 1, 1, 2, 2, 1, 1, 2);
}

TEST(Render, ZoomAndDstBounds)
{
  Context ctx;

  // Create this image:
  // 0 0 0
  // 0 4 4
  // 0 4 4
  Document* doc = ctx.documents().add(3, 3, ColorMode::RGB);
  Image* src = doc->sprite()->layer(0)->cel(0)->image();
  clear_image(src, 0);
  fill_rect(src, 1, 1, 2, 2, 4);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 4, 4));
  clear_image(dst.get(), 0);

  Render render;
  render.setBgType(BgType::CHECKED);
  render.setBgZoom(true);
  render.setBgColor1(1);
  render.setBgColor2(2);
  render.setBgCheckedSize(gfx::Size(1, 1));

  render.renderSprite(dst.get(), doc->sprite(), frame_t(0),
                      gfx::Clip(1, 1, 0, 0, 2, 2), Zoom(1, 1));
  EXPECT_4X4_PIXELS(dst.get(), 0, 0, 0, 0, 0, 1, 2, 0, 0, 2, 4, 0, 0, 0, 0, 0);
}

// Golden-image test for the checkered-background ordering in
// Render::renderSprite: the checkered pattern must be composited *after*
// the layers, onionskin and preview overlay (so blend functions see the
// real per-pixel alpha), yet still end up underneath all of them.
TEST(Render, CheckedBackgroundWithOnionskinAndPreviewGolden)
{
  Context ctx;
  Document* doc = ctx.documents().add(4, 4, ColorMode::RGB);
  Sprite* sprite = doc->sprite();
  sprite->setTotalFrames(2);

  const color_t transparent = rgba(0, 0, 0, 0);
  const color_t red = rgba(255, 0, 0, 255);
  const color_t halfGreen = rgba(0, 255, 0, 128);
  const color_t blue = rgba(0, 0, 255, 255);
  const color_t yellow = rgba(255, 255, 0, 255);

  LayerImage* layer = static_cast<LayerImage*>(sprite->layer(0));

  // Frame 0: an opaque and a half-transparent pixel, rest transparent.
  Image* img0 = layer->cel(0)->image();
  clear_image(img0, transparent);
  put_pixel(img0, 1, 1, red);
  put_pixel(img0, 2, 1, halfGreen);

  // Frame 1 (only visible through the onionskin): a blue pixel over the
  // transparent area and another hidden behind frame 0's opaque red one.
  ImageRef image1(Image::create(IMAGE_RGB, 4, 4));
  clear_image(image1.get(), transparent);
  put_pixel(image1.get(), 0, 0, blue);
  put_pixel(image1.get(), 1, 1, blue);
  layer->addCel(std::make_shared<Cel>(frame_t(1), image1));

  std::unique_ptr<Image> preview(Image::create(IMAGE_RGB, 1, 1));
  put_pixel(preview.get(), 0, 0, yellow);

  Render render;
  render.setBgType(BgType::CHECKED);
  render.setBgZoom(true);
  render.setBgColor1(rgba(10, 10, 10, 255));
  render.setBgColor2(rgba(200, 200, 200, 255));
  render.setBgCheckedSize(gfx::Size(1, 1));

  OnionskinOptions onion(OnionskinType::MERGE);
  onion.position(OnionskinPosition::BEHIND);
  onion.prevFrames(0);
  onion.nextFrames(1);
  onion.opacityBase(128);
  onion.opacityStep(0);
  render.setOnionskin(onion);
  render.setPreviewImage(nullptr, frame_t(0), preview.get(), gfx::Point(3, 3),
                         BlendMode::NORMAL);

  std::unique_ptr<Image> dst(Image::create(IMAGE_RGB, 4, 4));
  clear_image(dst.get(), 0);
  render.renderSprite(dst.get(), sprite, frame_t(0));

  const color_t d = rgba(10, 10, 10, 255);    // dark checker tile
  const color_t l = rgba(200, 200, 200, 255); // light checker tile
  const color_t onionBlue = rgba(5, 5, 132, 255); // 50% blue over d
  const color_t greenOnL = rgba(100, 227, 100, 255); // half green over l

  // Expected (golden) output; recorded from a run and checked by hand:
  // - (0,0): onionskin blue at 50% blends against the checker, not
  //          against transparency.
  // - (1,1): opaque red covers the onionskin's blue and the checker.
  // - (2,1): half-transparent green blends with the checker behind it.
  // - (3,3): the opaque preview overlay covers the checker.
  // - everything else is plain checkerboard.
  EXPECT_4X4_PIXELS(dst.get(),
                    onionBlue, l, d, l,
                    l, red, greenOnL, d,
                    d, l, d, l,
                    l, d, l, yellow);
}
