// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "doc/cel.h"
#include "doc/frame.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <vector>

namespace doc
{
class Sprite;
}

namespace app
{

class AseReader;
class FileOp;

// Converts the tilemaps of an .ase file into regular cel images. A tileset
// may come after the cels which use it, so the cels are created with a
// placeholder image and rasterize() draws them once the file has been read.
class AseTilemaps
{
public:
  void readTileset(AseReader& f, const doc::Sprite* sprite);

  // Reads a "compressed tilemap" cel of a layer which uses "tilesetId".
  std::shared_ptr<doc::Cel> readCel(AseReader& f, doc::frame_t frame, uint32_t tilesetId, const doc::Sprite* sprite);

  // Must be called when a cel gets its own copy of another cel's image.
  void addCopy(const doc::Cel* source, const doc::Cel* copy);

  void rasterize(FileOp* fop, const doc::Sprite* sprite);

private:
  struct Tileset
  {
    uint32_t tiles = 0;
    int tileWidth = 0;
    int tileHeight = 0;
    bool zeroIsEmpty = false;
    std::vector<uint8_t> pixels; // Empty if the tiles aren't in the file.
  };

  struct Tilemap
  {
    uint32_t tilesetId;
    int width; // In tiles
    int height;
    uint32_t idMask;
    uint32_t xFlipMask;
    uint32_t yFlipMask;
    uint32_t diagonalFlipMask;
    std::vector<uint8_t> compressed;
    std::vector<doc::CelDataRef> targets; // Cel data which need the image
  };

  doc::ImageRef render(FileOp* fop, const doc::Sprite* sprite, Tilemap& tilemap);

  std::map<uint32_t, Tileset> m_tilesets;
  std::vector<Tilemap> m_tilemaps;
  std::map<const doc::CelData*, std::size_t> m_tilemapOfData;
  uint64_t m_tilesetPixels = 0;
  bool m_warnedMissingTileset = false;
  bool m_warnedMissingTiles = false;
  bool m_warnedTileId = false;
};

} // namespace app
