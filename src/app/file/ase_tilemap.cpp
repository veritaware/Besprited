// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/file/ase_tilemap.h"

#include "app/file/ase_reader.h"
#include "app/file/file.h"
#include "base/exception.h"
#include "doc/doc.h"
#include "zlib.h"

namespace app
{

using namespace doc;

namespace
{

constexpr uint32_t kTilesetHasTiles = 2;
constexpr uint32_t kTilesetZeroIsEmpty = 4;
// Empty tile in files without kTilesetZeroIsEmpty.
constexpr uint32_t kOldEmptyTile = 0xffffffff;
constexpr uint64_t kMaxTilesetPixels = uint64_t(kMaxFileImageDimension) * kMaxFileImageDimension;
// Deflate can't expand its input more than this.
constexpr size_t kMaxDeflateRatio = 1032;

int bytesPerPixel(PixelFormat format)
{
  return format == IMAGE_RGB ? 4 : (format == IMAGE_GRAYSCALE ? 2 : 1);
}

std::vector<uint8_t> readBytes(AseReader& f, size_t size)
{
  f.require(size);
  std::vector<uint8_t> bytes(size);
  f.read(bytes.data(), size);
  return bytes;
}

// Fails unless the stream holds exactly "size" bytes. A missing zlib trailer
// is tolerated, as in compressed cels.
std::vector<uint8_t> inflateExact(std::vector<uint8_t>& input, size_t size)
{
  // Don't allocate a big buffer for a stream which can't fill it.
  if (size / kMaxDeflateRatio > input.size())
    throw base::Exception("Compressed ASE tile data is too short.");

  z_stream stream{};
  if (inflateInit(&stream) != Z_OK)
    throw base::Exception("Cannot initialize ASE tile decompression.");
  struct InflateGuard
  {
    z_stream* stream;
    ~InflateGuard() { inflateEnd(stream); }
  } guard{&stream};

  // One extra byte to detect a stream which is too long.
  std::vector<uint8_t> output(size + 1);
  stream.next_in = input.data();
  stream.avail_in = static_cast<uInt>(input.size());
  stream.next_out = output.data();
  stream.avail_out = static_cast<uInt>(output.size());

  const int result = inflate(&stream, Z_FINISH);
  if ((result != Z_STREAM_END && result != Z_OK && result != Z_BUF_ERROR) || stream.total_out != size)
    throw base::Exception("Invalid compressed ASE tile data.");

  output.resize(size);
  return output;
}

uint32_t readLE32(const uint8_t* p)
{
  return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}

color_t readColor(PixelFormat format, const uint8_t* p)
{
  switch (format)
  {
  case IMAGE_RGB:
    return rgba(p[0], p[1], p[2], p[3]);
  case IMAGE_GRAYSCALE:
    return graya(p[0], p[1]);
  default:
    return p[0];
  }
}

ImageRef createEmptyImage(const Sprite* sprite, int width, int height)
{
  ImageRef image(Image::create(sprite->pixelFormat(), width, height));
  image->setMaskColor(sprite->transparentColor());
  clear_image(image.get(), sprite->transparentColor());
  return image;
}

} // namespace

void AseTilemaps::readTileset(AseReader& f, const Sprite* sprite)
{
  const uint32_t id = f.dword();
  const uint32_t flags = f.dword();

  Tileset tileset;
  tileset.tiles = f.dword();
  tileset.tileWidth = f.word();
  tileset.tileHeight = f.word();
  tileset.zeroIsEmpty = (flags & kTilesetZeroIsEmpty) != 0;
  f.skip(16);       // Base index and reserved bytes
  (void)f.string(); // Name

  if (tileset.tileWidth == 0 || tileset.tileHeight == 0 || tileset.tileWidth > kMaxFileImageDimension ||
      tileset.tileHeight > kMaxFileImageDimension)
    throw base::Exception("Invalid ASE tile size.");

  if (flags & 1) // Link to an external file
    f.skip(8);

  if (flags & kTilesetHasTiles)
  {
    const uint64_t tilePixels = uint64_t(tileset.tileWidth) * tileset.tileHeight;
    if (tileset.tiles > (kMaxTilesetPixels - m_tilesetPixels) / tilePixels)
      throw base::Exception("ASE tilesets are too big.");

    const uint32_t length = f.dword();
    std::vector<uint8_t> compressed = readBytes(f, length);
    const uint64_t pixels = tilePixels * tileset.tiles;
    tileset.pixels = inflateExact(compressed, size_t(pixels * bytesPerPixel(sprite->pixelFormat())));
    m_tilesetPixels += pixels;
  }

  m_tilesets[id] = std::move(tileset);
}

std::shared_ptr<Cel> AseTilemaps::readCel(AseReader& f, frame_t frame, uint32_t tilesetId, const Sprite* sprite)
{
  Tilemap tilemap;
  tilemap.tilesetId = tilesetId;
  tilemap.width = f.word();
  tilemap.height = f.word();
  const int bitsPerTile = f.word();
  tilemap.idMask = f.dword();
  tilemap.xFlipMask = f.dword();
  tilemap.yFlipMask = f.dword();
  tilemap.diagonalFlipMask = f.dword();
  f.skip(10);

  if (bitsPerTile != 32)
    throw base::Exception("Unsupported ASE tile size of %d bits.", bitsPerTile);
  if (tilemap.width == 0 || tilemap.height == 0 || tilemap.width > kMaxFileImageDimension ||
      tilemap.height > kMaxFileImageDimension)
    throw base::Exception("Invalid ASE tilemap size.");

  tilemap.compressed = readBytes(f, f.remaining());

  auto cel = std::make_shared<Cel>(frame, createEmptyImage(sprite, 1, 1));
  tilemap.targets.push_back(cel->dataRef());
  m_tilemapOfData[cel->data()] = m_tilemaps.size();
  m_tilemaps.push_back(std::move(tilemap));
  return cel;
}

void AseTilemaps::addCopy(const Cel* source, const Cel* copy)
{
  const auto it = m_tilemapOfData.find(source->data());
  if (it == m_tilemapOfData.end())
    return;

  const std::size_t index = it->second;
  m_tilemaps[index].targets.push_back(copy->dataRef());
  m_tilemapOfData[copy->data()] = index;
}

void AseTilemaps::rasterize(FileOp* fop, const Sprite* sprite)
{
  for (Tilemap& tilemap : m_tilemaps)
  {
    if (fop->isStop())
      break;

    // A bad tilemap only loses its own cels.
    try
    {
      const ImageRef image = render(fop, sprite, tilemap);
      if (!image)
        continue;

      for (std::size_t i = 0; i < tilemap.targets.size(); ++i)
      {
        ImageRef target = image;
        if (i > 0)
          target.reset(Image::createCopy(image.get()));
        tilemap.targets[i]->setImage(target);
      }
    }
    catch (const std::exception& e)
    {
      fop->setError("Warning: %s\n", e.what());
    }
  }

  m_tilemaps.clear();
  m_tilemapOfData.clear();
}

ImageRef AseTilemaps::render(FileOp* fop, const Sprite* sprite, Tilemap& tilemap)
{
  const auto it = m_tilesets.find(tilemap.tilesetId);
  if (it == m_tilesets.end())
  {
    if (!m_warnedMissingTileset)
      fop->addWarning("Some tilemaps use a tileset which is not in the file; they are left empty.");
    m_warnedMissingTileset = true;
    return ImageRef();
  }

  const Tileset& tileset = it->second;
  const int tw = tileset.tileWidth;
  const int th = tileset.tileHeight;
  if (tilemap.width > kMaxFileImageDimension / tw || tilemap.height > kMaxFileImageDimension / th)
    throw base::Exception("ASE tilemap is too big.");

  const std::vector<uint8_t> tiles = inflateExact(tilemap.compressed, size_t(tilemap.width) * tilemap.height * 4);
  const ImageRef image = createEmptyImage(sprite, tilemap.width * tw, tilemap.height * th);

  if (tileset.pixels.empty())
  {
    if (!m_warnedMissingTiles)
      fop->addWarning(
          "Some tilesets have no tiles in the file (external tilesets are not supported); they are left empty.");
    m_warnedMissingTiles = true;
    return image;
  }

  const PixelFormat format = sprite->pixelFormat();
  const int bpp = bytesPerPixel(format);
  const size_t tileBytes = size_t(tw) * th * bpp;

  for (int row = 0; row < tilemap.height; ++row)
  {
    for (int col = 0; col < tilemap.width; ++col)
    {
      const uint32_t tile = readLE32(&tiles[(size_t(row) * tilemap.width + col) * 4]);
      const uint32_t id = tile & tilemap.idMask;
      if (tileset.zeroIsEmpty ? id == 0 : tile == kOldEmptyTile)
        continue;
      if (id >= tileset.tiles)
      {
        if (!m_warnedTileId)
          fop->addWarning("Some tilemaps use tiles which are not in their tileset; they are left empty.");
        m_warnedTileId = true;
        continue;
      }

      // The spec doesn't define the order of the flips. This is the usual
      // one (also Tiled's): diagonal, then X, then Y.
      const bool xFlip = (tile & tilemap.xFlipMask) != 0;
      const bool yFlip = (tile & tilemap.yFlipMask) != 0;
      const bool diagonalFlip = (tile & tilemap.diagonalFlipMask) != 0;
      const uint8_t* source = &tileset.pixels[id * tileBytes];

      for (int y = 0; y < th; ++y)
      {
        for (int x = 0; x < tw; ++x)
        {
          int u = xFlip ? tw - 1 - x : x;
          int v = yFlip ? th - 1 - y : y;
          if (diagonalFlip)
            std::swap(u, v);
          // Transposed non-square tiles are clipped to the tile cell.
          if (u >= tw || v >= th)
            continue;

          image->putPixel(col * tw + x, row * th + y, readColor(format, source + (size_t(v) * tw + u) * bpp));
        }
      }
    }
  }

  return image;
}

} // namespace app
