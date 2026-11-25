// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/context.h"
#include "app/document.h"
#include "app/file/file.h"
#include "doc/doc.h"
#include "she/system.h"
#include "zlib.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

using namespace app;

namespace
{

constexpr int kTile = 2; // Tile width and height used by most tests
constexpr uint32_t kXFlip = 0x20000000;
constexpr uint32_t kYFlip = 0x40000000;
constexpr uint32_t kDiagonalFlip = 0x80000000;
constexpr uint32_t kExternal = 1;
constexpr uint32_t kEmbedded = 2;
constexpr uint32_t kZeroIsEmpty = 4;

// Little-endian writer for the parts of an .ase file.
struct Bytes
{
  std::vector<uint8_t> data;

  Bytes& u8(uint32_t v)
  {
    data.push_back(uint8_t(v));
    return *this;
  }
  Bytes& u16(uint32_t v) { return u8(v).u8(v >> 8); }
  Bytes& u32(uint32_t v) { return u16(v).u16(v >> 16); }
  Bytes& zeros(size_t n)
  {
    data.insert(data.end(), n, 0);
    return *this;
  }
  Bytes& add(const std::vector<uint8_t>& v)
  {
    data.insert(data.end(), v.begin(), v.end());
    return *this;
  }
  Bytes& text(const std::string& s)
  {
    u16(uint32_t(s.size()));
    data.insert(data.end(), s.begin(), s.end());
    return *this;
  }
};

std::vector<uint8_t> deflateBytes(const std::vector<uint8_t>& input)
{
  uLongf size = compressBound(input.size());
  std::vector<uint8_t> result(size);
  EXPECT_EQ(Z_OK, compress(result.data(), &size, input.data(), input.size()));
  result.resize(size);
  return result;
}

std::vector<uint8_t> chunk(int type, const Bytes& payload)
{
  Bytes result;
  result.u32(uint32_t(payload.data.size() + 6)).u16(type).add(payload.data);
  return result.data;
}

std::vector<uint8_t> layerChunk(int type, int childLevel, const std::string& name, uint32_t tileset = 0,
                                int blendMode = 0, int opacity = 255)
{
  Bytes b;
  b.u16(3).u16(type).u16(childLevel).u16(0).u16(0).u16(blendMode).u8(opacity).zeros(3).text(name);
  if (type == 2)
    b.u32(tileset);
  return chunk(0x2004, b);
}

Bytes celHeader(int layer, int x, int y, int type, int opacity = 255)
{
  Bytes b;
  b.u16(layer).u16(x).u16(y).u8(opacity).u16(type).zeros(7);
  return b;
}

std::vector<uint8_t> imageCel(int layer, int w, int h, const std::vector<uint8_t>& pixels)
{
  Bytes b = celHeader(layer, 0, 0, 2);
  b.u16(w).u16(h).add(deflateBytes(pixels));
  return chunk(0x2005, b);
}

std::vector<uint8_t> linkCel(int layer, int frame, int x = 0, int y = 0)
{
  Bytes b = celHeader(layer, x, y, 1);
  b.u16(frame);
  return chunk(0x2005, b);
}

std::vector<uint8_t> rawTilemapCel(int layer, int w, int h, const std::vector<uint8_t>& compressed, int x = 0,
                                   int y = 0)
{
  Bytes b = celHeader(layer, x, y, 3);
  b.u16(w).u16(h).u16(32).u32(0x1fffffff).u32(kXFlip).u32(kYFlip).u32(kDiagonalFlip).zeros(10).add(compressed);
  return chunk(0x2005, b);
}

std::vector<uint8_t> tileBytes(const std::vector<uint32_t>& tiles)
{
  Bytes b;
  for (uint32_t tile : tiles)
    b.u32(tile);
  return b.data;
}

std::vector<uint8_t> tilemapCel(int layer, int w, int h, const std::vector<uint32_t>& tiles, int x = 0, int y = 0)
{
  return rawTilemapCel(layer, w, h, deflateBytes(tileBytes(tiles)), x, y);
}

std::vector<uint8_t> tilesetChunk(uint32_t id, uint32_t flags, uint32_t count, int w, int h,
                                  const std::vector<uint8_t>& pixels = {})
{
  Bytes b;
  b.u32(id).u32(flags).u32(count).u16(w).u16(h).u16(1).zeros(14).text("tiles");
  if (flags & kExternal)
    b.u32(1).u32(0);
  if (flags & kEmbedded)
  {
    const auto compressed = deflateBytes(pixels);
    b.u32(uint32_t(compressed.size())).add(compressed);
  }
  return chunk(0x2023, b);
}

std::vector<uint8_t> userDataChunk(const std::string& text)
{
  Bytes b;
  b.u32(1).text(text);
  return chunk(0x2020, b);
}

// Pixel "n" of the test images: a distinct opaque color.
doc::color_t color(int n)
{
  return doc::rgba(n * 10, n * 5, 200, 255);
}

// RGBA bytes of the given colors, 0 standing for a transparent pixel.
std::vector<uint8_t> pixels(const std::vector<int>& numbers)
{
  Bytes b;
  for (int n : numbers)
    b.u32(n ? color(n) : 0);
  return b.data;
}

// Tile 0 is {9,9,9,9}, tile 1 is {1,2,3,4}.
std::vector<uint8_t> twoTiles(uint32_t flags = kEmbedded | kZeroIsEmpty)
{
  return tilesetChunk(0, flags, 2, kTile, kTile, pixels({9, 9, 9, 9, 1, 2, 3, 4}));
}

using Frame = std::vector<std::vector<uint8_t>>;

struct Loaded
{
  app::Context ctx;
  std::unique_ptr<app::Document> document;
  std::string error;
  std::vector<std::string> warnings;

  ~Loaded() { close(); }

  // A document must leave its context before it's deleted.
  void close()
  {
    if (document)
      document->close();
    document.reset();
  }

  doc::Layer* layer(int i) { return document->sprite()->indexToLayer(doc::LayerIndex(i)); }
  std::shared_ptr<doc::Cel> cel(int i, int frame = 0) { return layer(i)->cel(doc::frame_t(frame)); }

  // Pixels of a cel as a string of color() numbers, '.' being transparent.
  std::string draw(int i, int frame = 0)
  {
    const auto c = cel(i, frame);
    if (!c)
      return "no cel";
    std::string result;
    for (int y = 0; y < c->image()->height(); ++y)
    {
      if (y)
        result += '/';
      for (int x = 0; x < c->image()->width(); ++x)
      {
        const doc::color_t pixel = doc::get_pixel(c->image(), x, y);
        char value = pixel ? '?' : '.';
        for (int n = 1; n <= 9; ++n)
          if (pixel == color(n))
            value = char('0' + n);
        result += value;
      }
    }
    return result;
  }
};

class AseTilemap : public ::testing::Test
{
protected:
  std::unique_ptr<she::System> sys{she::create_system()};
  const char* filename = "ase_tilemap_test.ase";
  ~AseTilemap() override { std::remove(filename); }

  std::unique_ptr<Loaded> load(const std::vector<Frame>& frames, int depth = 32, int transparent = 0)
  {
    Bytes body;
    for (const Frame& frame : frames)
    {
      Bytes chunks;
      for (const auto& c : frame)
        chunks.add(c);
      body.u32(uint32_t(chunks.data.size() + 16)).u16(0xf1fa).u16(uint32_t(frame.size()));
      body.u16(100).zeros(2).u32(uint32_t(frame.size())).add(chunks.data);
    }
    Bytes file;
    file.u32(uint32_t(body.data.size() + 128)).u16(0xa5e0).u16(uint32_t(frames.size()));
    file.u16(16).u16(16).u16(depth).u32(1).u16(100).zeros(8).u8(transparent).zeros(3).u16(256);
    file.zeros(128 - file.data.size()).add(body.data);

    std::ofstream(filename, std::ios::binary).write(reinterpret_cast<const char*>(file.data.data()), file.data.size());

    auto result = std::make_unique<Loaded>();
    std::unique_ptr<FileOp> fop(FileOp::createLoadDocumentOperation(&result->ctx, filename, FILE_LOAD_SEQUENCE_NONE));
    fop->operate();
    fop->done();
    fop->postLoad();
    result->error = fop->error();
    result->warnings = fop->warnings();
    result->document.reset(fop->releaseDocument());
    return result;
  }
};

bool hasWarning(const Loaded& loaded, const std::string& part)
{
  return std::any_of(loaded.warnings.begin(), loaded.warnings.end(),
                     [&](const std::string& w) { return w.find(part) != std::string::npos; });
}

} // namespace

TEST_F(AseTilemap, TilemapLayerBecomesImageLayerAndKeepsTheLayersAfterIt)
{
  auto loaded =
      load({{twoTiles(), userDataChunk("tileset data"), layerChunk(2, 0, "map", 0, 1, 128), layerChunk(0, 0, "image"),
             tilemapCel(0, 2, 1, {1, 1}, 3, 4), imageCel(1, 2, 1, pixels({5, 6}))}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  ASSERT_EQ(2, int(loaded->document->sprite()->countLayers()));
  ASSERT_TRUE(loaded->layer(0)->isImage());
  auto* map = static_cast<doc::LayerImage*>(loaded->layer(0));
  EXPECT_EQ("map", map->name());
  EXPECT_EQ(128, map->opacity());
  EXPECT_EQ(doc::BlendMode::MULTIPLY, map->blendMode());
  EXPECT_EQ("", map->userData().text());
  EXPECT_EQ("1212/3434", loaded->draw(0));
  EXPECT_EQ(3, loaded->cel(0)->x());
  EXPECT_EQ(4, loaded->cel(0)->y());
  EXPECT_EQ("56", loaded->draw(1));
  ASSERT_EQ(1u, loaded->warnings.size());
  EXPECT_TRUE(hasWarning(*loaded, "Tilemap layers were converted"));
}

TEST_F(AseTilemap, AppliesEveryFlipCombination)
{
  std::vector<uint32_t> tiles;
  for (uint32_t flips = 0; flips < 8; ++flips)
    tiles.push_back(1 | (flips & 1 ? kXFlip : 0) | (flips & 2 ? kYFlip : 0) | (flips & 4 ? kDiagonalFlip : 0));

  auto loaded = load({{twoTiles(), layerChunk(2, 0, "map"), tilemapCel(0, 8, 1, tiles)}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  //         none X  Y  XY D  DX DY DXY
  EXPECT_EQ("1221344313312442/3443122124421331", loaded->draw(0));
}

TEST_F(AseTilemap, ClipsTransposedNonSquareTiles)
{
  auto loaded = load({{tilesetChunk(0, kEmbedded | kZeroIsEmpty, 2, 2, 1, pixels({9, 9, 1, 2})),
                       layerChunk(2, 0, "map"), tilemapCel(0, 1, 1, {1 | kDiagonalFlip})}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("1.", loaded->draw(0));
}

TEST_F(AseTilemap, TileZeroIsEmptyWhenTheTilesetSaysSo)
{
  auto loaded = load({{twoTiles(), layerChunk(2, 0, "map"), tilemapCel(0, 2, 1, {0, 1})}});

  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("..12/..34", loaded->draw(0));
}

TEST_F(AseTilemap, OldEmptyTileIsUsedWithoutTheFlag)
{
  auto loaded = load({{twoTiles(kEmbedded), layerChunk(2, 0, "map"), tilemapCel(0, 2, 1, {0, 0xffffffff})}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("99../99..", loaded->draw(0));
  EXPECT_EQ(1u, loaded->warnings.size());
}

TEST_F(AseTilemap, TilesetMayFollowTheCels)
{
  auto loaded = load({{layerChunk(2, 0, "map"), tilemapCel(0, 1, 1, {1}), twoTiles()}});

  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("12/34", loaded->draw(0));
}

TEST_F(AseTilemap, LinkedCelsGetThePixels)
{
  auto loaded = load({{twoTiles(), layerChunk(2, 0, "map"), tilemapCel(0, 1, 1, {1}), userDataChunk("cel")},
                      {linkCel(0, 0)},
                      {linkCel(0, 0, 5, 6)},
                      {linkCel(0, 2, 5, 6)}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  for (int frame = 0; frame < 4; ++frame)
    EXPECT_EQ("12/34", loaded->draw(0, frame)) << frame;
  EXPECT_EQ(loaded->cel(0, 0)->data(), loaded->cel(0, 1)->data());
  EXPECT_NE(loaded->cel(0, 0)->data(), loaded->cel(0, 2)->data());
  EXPECT_EQ(loaded->cel(0, 2)->data(), loaded->cel(0, 3)->data());
  EXPECT_EQ(5, loaded->cel(0, 2)->x());
  EXPECT_EQ("cel", loaded->cel(0, 0)->data()->userData().text());
}

TEST_F(AseTilemap, EmptyAreasOfIndexedSpritesUseTheTransparentColor)
{
  auto loaded = load({{tilesetChunk(0, kEmbedded | kZeroIsEmpty, 2, 2, 1, {0, 0, 1, 2}), layerChunk(2, 0, "map"),
                       tilemapCel(0, 2, 1, {0, 1})}},
                     8, 7);

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  const doc::Image* image = loaded->cel(0)->image();
  ASSERT_EQ(4, image->width());
  EXPECT_EQ(7, doc::get_pixel(image, 0, 0));
  EXPECT_EQ(7, doc::get_pixel(image, 1, 0));
  EXPECT_EQ(1, doc::get_pixel(image, 2, 0));
  EXPECT_EQ(2, doc::get_pixel(image, 3, 0));
  EXPECT_EQ(7, image->maskColor());
}

TEST_F(AseTilemap, DrawsGrayscaleTiles)
{
  auto loaded = load({{tilesetChunk(0, kEmbedded | kZeroIsEmpty, 2, 1, 1, {0, 0, 40, 255}), layerChunk(2, 0, "map"),
                       tilemapCel(0, 1, 1, {1})}},
                     16);

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ(doc::graya(40, 255), doc::get_pixel(loaded->cel(0)->image(), 0, 0));
}

TEST_F(AseTilemap, LoadsHighlyCompressibleTileData)
{
  const int size = 1024;
  auto loaded = load({{tilesetChunk(0, kEmbedded | kZeroIsEmpty, 1, 1, 1, pixels({0})), layerChunk(2, 0, "map"),
                       tilemapCel(0, size, size, std::vector<uint32_t>(size * size, 0))}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ(size, loaded->cel(0)->image()->width());
}

TEST_F(AseTilemap, KeepsGroupNesting)
{
  auto loaded = load({{twoTiles(), layerChunk(1, 0, "G"), layerChunk(2, 1, "map"), layerChunk(0, 1, "inside"),
                       layerChunk(0, 0, "outside"), tilemapCel(1, 1, 1, {1}), imageCel(2, 1, 1, pixels({5})),
                       imageCel(3, 1, 1, pixels({6}))}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  // The group is flattened after loading; the names show the nesting.
  ASSERT_EQ(3, int(loaded->document->sprite()->countLayers()));
  EXPECT_EQ("G-map", loaded->layer(0)->name());
  EXPECT_EQ("G-inside", loaded->layer(1)->name());
  EXPECT_EQ("outside", loaded->layer(2)->name());
  EXPECT_EQ("12/34", loaded->draw(0));
  EXPECT_EQ("5", loaded->draw(1));
  EXPECT_EQ("6", loaded->draw(2));
}

TEST_F(AseTilemap, ExternalTilesetGivesEmptyTilesAndAWarning)
{
  auto loaded = load({{tilesetChunk(0, kExternal | kZeroIsEmpty, 2, kTile, kTile), layerChunk(2, 0, "map"),
                       tilemapCel(0, 1, 1, {1})}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("../..", loaded->draw(0));
  EXPECT_TRUE(hasWarning(*loaded, "external tilesets"));
}

TEST_F(AseTilemap, SkippableChunksDoNotFailTheLoad)
{
  auto loaded =
      load({{layerChunk(0, 0, "image"), chunk(0x2022, Bytes().zeros(20)), userDataChunk("slice"),
             chunk(0x2008, Bytes().zeros(12)), imageCel(0, 1, 1, pixels({5})), chunk(0x2006, Bytes().zeros(36))}});

  EXPECT_EQ("", loaded->error);
  EXPECT_TRUE(loaded->warnings.empty());
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("", loaded->layer(0)->userData().text());
  EXPECT_EQ("5", loaded->draw(0));
}

TEST_F(AseTilemap, UnsupportedLayerTypeKeepsTheLayerIndexes)
{
  auto loaded = load({{layerChunk(7, 0, "future"), layerChunk(0, 0, "image"), imageCel(0, 1, 1, pixels({4})),
                       imageCel(1, 1, 1, pixels({5}))}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  ASSERT_EQ(1, int(loaded->document->sprite()->countLayers()));
  EXPECT_EQ("5", loaded->draw(0));
  EXPECT_TRUE(hasWarning(*loaded, "unsupported type"));
}

TEST_F(AseTilemap, TileOutsideTheTilesetIsEmpty)
{
  auto loaded = load({{twoTiles(), layerChunk(2, 0, "map"), tilemapCel(0, 2, 1, {2, 1})}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("..12/..34", loaded->draw(0));
  EXPECT_TRUE(hasWarning(*loaded, "not in their tileset"));
}

TEST_F(AseTilemap, MissingTilesetLeavesTheCelEmpty)
{
  auto loaded = load({{twoTiles(), layerChunk(2, 0, "map", 5), layerChunk(0, 0, "image"), tilemapCel(0, 1, 1, {1}),
                       imageCel(1, 1, 1, pixels({5}))}});

  EXPECT_EQ("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ(".", loaded->draw(0));
  EXPECT_EQ("5", loaded->draw(1));
  EXPECT_TRUE(hasWarning(*loaded, "not in the file"));
}

namespace
{

class AseBadTilemap : public AseTilemap,
                      public ::testing::WithParamInterface<int>
{
protected:
  // A broken tileset or tilemap cel.
  static Frame badChunks(int which)
  {
    switch (which)
    {
    case 0: // Too few tiles
      return {twoTiles(), rawTilemapCel(0, 2, 1, deflateBytes(tileBytes({1})))};
    case 1: // Too many tiles
      return {twoTiles(), rawTilemapCel(0, 1, 1, deflateBytes(tileBytes({1, 1})))};
    case 2: // Not a zlib stream
      return {twoTiles(), rawTilemapCel(0, 1, 1, {1, 2, 3, 4})};
    case 3: // Grid above the image size limit
      return {twoTiles(), tilemapCel(0, 0xffff, 0xffff, {1})};
    case 4: // Grid within the limit, but not once multiplied by the tile size
      return {twoTiles(), tilemapCel(0, 10000, 1, {1})};
    case 5: // Tileset pixels of the wrong size
      return {tilesetChunk(0, kEmbedded, 3, kTile, kTile, pixels({1, 2, 3, 4})), tilemapCel(0, 1, 1, {0})};
    case 6: // Huge tileset
      return {tilesetChunk(0, kEmbedded, 0xffffffff, 0x4000, 0x4000, pixels({1})), tilemapCel(0, 1, 1, {0})};
    case 7: // Zero tile size
      return {tilesetChunk(0, kEmbedded, 1, 0, 0), tilemapCel(0, 1, 1, {0})};
    case 8: // Tileset far bigger than its data could expand to
      return {tilesetChunk(0, kEmbedded, 1, 0x4000, 0x4000, pixels({1})), tilemapCel(0, 1, 1, {0})};
    default: // Truncated cel
      return {twoTiles(), chunk(0x2005, celHeader(0, 0, 0, 3))};
    }
  }
};

} // namespace

TEST_P(AseBadTilemap, ReportsAnErrorAndKeepsTheOtherLayers)
{
  Frame frame = {layerChunk(2, 0, "map"), layerChunk(0, 0, "image"), imageCel(1, 1, 1, pixels({5}))};
  for (auto& c : badChunks(GetParam()))
    frame.push_back(c);

  auto loaded = load({frame});

  EXPECT_NE("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("5", loaded->draw(1));
}

INSTANTIATE_TEST_SUITE_P(Malformed, AseBadTilemap, ::testing::Range(0, 10));

TEST_F(AseTilemap, TilemapCelInAnImageLayerIsRejected)
{
  auto loaded = load({{twoTiles(), layerChunk(0, 0, "image"), tilemapCel(0, 1, 1, {1})}});

  EXPECT_NE("", loaded->error);
  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ(nullptr, loaded->cel(0));
}

TEST_F(AseTilemap, ConvertedLayersSurviveSavingAndReloading)
{
  auto loaded = load({{twoTiles(), layerChunk(2, 0, "map"), tilemapCel(0, 1, 1, {1})}});
  ASSERT_NE(nullptr, loaded->document);

  loaded->document->setContext(&loaded->ctx);
  ASSERT_EQ(0, save_document(&loaded->ctx, loaded->document.get()));
  loaded->close();
  loaded->document.reset(load_document(&loaded->ctx, filename));

  ASSERT_NE(nullptr, loaded->document);
  EXPECT_EQ("12/34", loaded->draw(0));
}
