#include "tests/test.h"

#include "app/util/freetype_utils.h"
#include "base/fs.h"
#include "base/path.h"
#include "doc/color.h"
#include "doc/image.h"
#include "doc/primitives.h"
#include "ft/face.h"
#include "ft/lib.h"
#include "gfx/rect.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

using namespace app;

namespace {

std::string bundled_font_path()
{
  auto srcDir = base::normalize_path(base::join_path(base::get_file_path(__FILE__), ".."));
  auto repoDir = base::normalize_path(base::join_path(srcDir, ".."));
  return base::join_path(repoDir, "data/fonts/pixeloid-sans.ttf");
}

std::pair<int, int> opaque_rows_in_range(const doc::Image* image, const gfx::Rect& glyphBounds)
{
  int top = image->height();
  int bottom = -1;

  for (int y=0; y<image->height(); ++y) {
    for (int x=glyphBounds.x; x<glyphBounds.x+glyphBounds.w; ++x) {
      if (doc::rgba_geta(doc::get_pixel(image, x, y)) > 0) {
        top = std::min(top, y);
        bottom = std::max(bottom, y);
      }
    }
  }

  return { top, bottom };
}

} // namespace

TEST(TextRender, UsesGlyphBaselineMetrics)
{
  const std::string text = "Ag";
  const std::string fontFile = bundled_font_path();
  ASSERT_TRUE(base::is_file(fontFile));

  ft::Lib ft;
  ft::Face face(ft.open(fontFile));
  ASSERT_TRUE(face.isValid());

  face.setSize(16);
  face.setAntialias(true);

  gfx::Rect expectedBounds(0, 0, 0, 0);
  std::vector<gfx::Rect> glyphBounds;
  face.forEachGlyph(text, [&](const ft::Glyph& glyph) {
    gfx::Rect glyphRect(int(glyph.x + glyph.bearingX),
                        int(glyph.y + glyph.offsetY),
                        int(glyph.bitmap->width),
                        int(glyph.bitmap->rows));
    expectedBounds |= glyphRect;
    glyphBounds.push_back(glyphRect);
  });

  ASSERT_EQ(2u, glyphBounds.size());

  gfx::Rect measuredBounds = face.calcTextBounds(text);
  EXPECT_EQ(expectedBounds.x, measuredBounds.x);
  EXPECT_EQ(expectedBounds.y, measuredBounds.y);
  EXPECT_EQ(expectedBounds.w, measuredBounds.w);
  EXPECT_EQ(expectedBounds.h, measuredBounds.h);

  std::unique_ptr<doc::Image> image(
    render_text(fontFile, 16, text, doc::rgba(255, 255, 255, 255), true));
  ASSERT_TRUE(image);
  EXPECT_EQ(expectedBounds.w, image->width());
  EXPECT_EQ(expectedBounds.h, image->height());

  gfx::Rect aBounds(glyphBounds[0].x - expectedBounds.x,
                    glyphBounds[0].y - expectedBounds.y,
                    glyphBounds[0].w,
                    glyphBounds[0].h);
  gfx::Rect gBounds(glyphBounds[1].x - expectedBounds.x,
                    glyphBounds[1].y - expectedBounds.y,
                    glyphBounds[1].w,
                    glyphBounds[1].h);

  const auto [aTop, aBottom] = opaque_rows_in_range(image.get(), aBounds);
  const auto [gTop, gBottom] = opaque_rows_in_range(image.get(), gBounds);

  ASSERT_GE(aTop, 0);
  ASSERT_GE(aBottom, 0);
  ASSERT_GE(gTop, 0);
  ASSERT_GE(gBottom, 0);

  EXPECT_LT(aTop, gTop);
  EXPECT_LT(aBottom, gBottom);
}
