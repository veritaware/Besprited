// Aseprite    | Copyright (C) 2001-2016 David Capello
// LibreSprite | Copyright (C) 2021-2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/context.h"
#include "app/document.h"
#include "app/file/file.h"
#include "app/file/ase_reader.h"
#include "app/file/file_format.h"
#include "app/file/format_options.h"
#include "base/cfile.h"
#include "base/exception.h"
#include "base/file_handle.h"
#include "base/path.h"
#include "doc/doc.h"
#include "ui/alert.h"
#include "zlib.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <memory>

#define ASE_FILE_MAGIC 0xA5E0
#define ASE_FILE_FRAME_MAGIC 0xF1FA

#define ASE_FILE_FLAG_LAYER_WITH_OPACITY 1

#define ASE_FILE_CHUNK_FLI_COLOR2 4
#define ASE_FILE_CHUNK_FLI_COLOR 11

#define ASE_FILE_CHUNK_LAYER 0x2004
#define ASE_FILE_CHUNK_CEL 0x2005
#define ASE_FILE_CHUNK_CEL_EXTRA 0x2006
#define ASE_FILE_CHUNK_COLOR_PROFILE 0x2007
#define ASE_FILE_CHUNK_EXTERNAL_FILE 0x2008
#define ASE_FILE_CHUNK_MASK 0x2016
#define ASE_FILE_CHUNK_PATH 0x2017
#define ASE_FILE_CHUNK_TAGS 0x2018
#define ASE_FILE_CHUNK_PALETTE 0x2019
#define ASE_FILE_CHUNK_USER_DATA 0x2020
#define ASE_FILE_CHUNK_SLICES                                                                                          \
  0x2021 // Deprecated chunk (used on dev versions only between v1.2-beta7 and
         // v1.2-beta8)
#define ASE_FILE_CHUNK_SLICE 0x2022
#define ASE_FILE_CHUNK_TILESET 0x2023

#define ASE_FILE_RAW_CEL 0
#define ASE_FILE_LINK_CEL 1
#define ASE_FILE_COMPRESSED_CEL 2

#define ASE_LAYER_FLAG_VISIBLE 1
#define ASE_LAYER_FLAG_EDITABLE 2
#define ASE_LAYER_FLAG_LOCK_MOVEMENT 4
#define ASE_LAYER_FLAG_BACKGROUND 8
#define ASE_LAYER_FLAG_PREFER_LINKED_CELS 16

#define ASE_PALETTE_FLAG_HAS_NAME 1

#define ASE_USER_DATA_FLAG_HAS_TEXT 1
#define ASE_USER_DATA_FLAG_HAS_COLOR 2

namespace app
{

using namespace base;

struct ASE_Header
{
  long pos;

  uint32_t size;
  uint16_t magic;
  uint16_t frames;
  uint16_t width;
  uint16_t height;
  uint16_t depth;
  uint32_t flags;
  uint16_t speed; // Deprecated, use "duration" of FrameHeader
  uint32_t next;
  uint32_t frit;
  uint8_t transparent_index;
  uint8_t ignore[3];
  uint16_t ncolors;
};

struct ASE_FrameHeader
{
  uint32_t size;
  uint16_t magic;
  uint32_t chunks;
  uint16_t duration;
};

struct ASE_Chunk
{
  int type;
  int start;
};

static bool ase_file_read_header(FILE* f, ASE_Header* header);
static void ase_file_prepare_header(FILE* f, ASE_Header* header, const Sprite* sprite);
static void ase_file_write_header(FILE* f, ASE_Header* header);
static void ase_file_write_header_filesize(FILE* f, ASE_Header* header);

static bool ase_file_read_frame_header(FILE* f, ASE_FrameHeader* frame_header);
static void ase_file_prepare_frame_header(FILE* f, ASE_FrameHeader* frame_header);
static void ase_file_write_frame_header(FILE* f, ASE_FrameHeader* frame_header);

static void ase_file_write_layers(FILE* f, ASE_FrameHeader* frame_header, const Layer* layer);
static void ase_file_write_cels(FILE* f, ASE_FrameHeader* frame_header, const Sprite* sprite, const Layer* layer,
                                frame_t frame);

static void ase_file_write_padding(FILE* f, int bytes);
static void ase_file_write_string(FILE* f, const std::string& string);

static void ase_file_write_start_chunk(FILE* f, ASE_FrameHeader* frame_header, int type, ASE_Chunk* chunk);
static void ase_file_write_close_chunk(FILE* f, ASE_Chunk* chunk);

static std::shared_ptr<Palette> ase_file_read_color_chunk(AseReader& f, const Palette& prevPal, frame_t frame);
static std::shared_ptr<Palette> ase_file_read_color2_chunk(AseReader& f, const Palette& prevPal, frame_t frame);
static std::shared_ptr<Palette> ase_file_read_palette_chunk(AseReader& f, const Palette& prevPal, frame_t frame);
static void ase_file_write_color2_chunk(FILE* f, ASE_FrameHeader* frame_header, const Palette* pal);
static void ase_file_write_palette_chunk(FILE* f, ASE_FrameHeader* frame_header, const Palette* pal, int from, int to);
static Layer* ase_file_read_layer_chunk(AseReader& f, ASE_Header* header, Sprite* sprite, Layer** previous_layer,
                                        int* current_level);
static void ase_file_write_layer_chunk(FILE* f, ASE_FrameHeader* frame_header, const Layer* layer);
static Cel* ase_file_read_cel_chunk(AseReader& f, Sprite* sprite, frame_t frame, PixelFormat pixelFormat, FileOp* fop,
                                    ASE_Header* header);
static void ase_file_write_cel_chunk(FILE* f, ASE_FrameHeader* frame_header, const Cel* cel, const LayerImage* layer,
                                     const Sprite* sprite);
static Mask* ase_file_read_mask_chunk(AseReader& f);
#if 0
static void ase_file_write_mask_chunk(FILE* f, ASE_FrameHeader* frame_header, Mask* mask);
#endif
static void ase_file_read_frame_tags_chunk(AseReader& f, FrameTags* frameTags);
static void ase_file_write_frame_tags_chunk(FILE* f, ASE_FrameHeader* frame_header, const FrameTags* frameTags);
static void ase_file_read_user_data_chunk(AseReader& f, UserData* userData);
static void ase_file_write_user_data_chunk(FILE* f, ASE_FrameHeader* frame_header, const UserData* userData);
static bool ase_has_groups(LayerFolder* layer);
static void ase_ungroup_all(LayerFolder* layer);

class ChunkWriter
{
public:
  ChunkWriter(FILE* f, ASE_FrameHeader* frame_header, int type)
    : m_file(f)
  {
    ase_file_write_start_chunk(m_file, frame_header, type, &m_chunk);
  }

  ~ChunkWriter() { ase_file_write_close_chunk(m_file, &m_chunk); }

private:
  FILE* m_file;
  ASE_Chunk m_chunk;
};

class AseFormat : public FileFormat
{
  const char* onGetName() const override { return "ase"; }
  const char* onGetExtensions() const override { return "ase,aseprite"; }
  int onGetFlags() const override
  {
    return FILE_SUPPORT_LOAD | FILE_SUPPORT_SAVE | FILE_SUPPORT_RGB | FILE_SUPPORT_RGBA | FILE_SUPPORT_GRAY |
           FILE_SUPPORT_GRAYA | FILE_SUPPORT_INDEXED | FILE_SUPPORT_LAYERS | FILE_SUPPORT_FRAMES |
           FILE_SUPPORT_PALETTES | FILE_SUPPORT_FRAME_TAGS | FILE_SUPPORT_BIG_PALETTES |
           FILE_SUPPORT_PALETTE_WITH_ALPHA;
  }

  bool onLoad(FileOp* fop) override;
  bool onPostLoad(FileOp* fop) override;
  bool onSave(FileOp* fop) override;
  int listPriority() override { return -100; }
};

static FileFormat::Regular<AseFormat> ff{"ase"};

bool AseFormat::onLoad(FileOp* fop)
{
  FileHandle handle(open_file_with_exception(fop->filename(), "rb"));
  FILE* f = handle.get();
  bool ignore_old_color_chunks = false;
  bool warned_unknown_chunk = false;
  if (fseek(f, 0, SEEK_END) != 0)
  {
    fop->setError("Error seeking in ASE file\n");
    return false;
  }
  const long file_end = ftell(f);
  if (file_end < 128 || fseek(f, 0, SEEK_SET) != 0)
  {
    fop->setError("Invalid ASE file size\n");
    return false;
  }

  ASE_Header header;
  if (!ase_file_read_header(f, &header))
  {
    fop->setError("Error reading header\n");
    return false;
  }

  if (header.width == 0 || header.height == 0 || header.width > kMaxFileImageDimension ||
      header.height > kMaxFileImageDimension)
  {
    fop->setError("Invalid ASE file: bad width/height\n");
    return false;
  }

  // Create the new sprite
  std::unique_ptr<Sprite> sprite(new Sprite(header.depth == 32   ? IMAGE_RGB
                                            : header.depth == 16 ? IMAGE_GRAYSCALE
                                                                 : IMAGE_INDEXED,
                                            header.width, header.height, header.ncolors));

  // Set frames and speed
  sprite->setTotalFrames(frame_t(header.frames));
  sprite->setDurationForAllFrames(header.speed);

  // Set transparent entry
  sprite->setTransparentColor(header.transparent_index);

  // Prepare variables for layer chunks
  Layer* last_layer = sprite->folder();
  WithUserData* last_object_with_user_data = nullptr;
  int current_level = -1;

  // Read frame by frame to end-of-file
  for (frame_t frame(0); frame < sprite->totalFrames(); ++frame)
  {
    // Start frame position
    const long frame_pos = ftell(f);
    fop->setProgress((float)frame_pos / (float)header.size);

    // Read frame header
    ASE_FrameHeader frame_header;
    // Without a valid frame header the next frame can't be located; keep the frames read so far.
    if (frame_pos < 0 || frame_pos > file_end || file_end - frame_pos < 16 ||
        !ase_file_read_frame_header(f, &frame_header))
    {
      fop->setError("Truncated ASE frame header in frame %d\n", (int)frame);
      break;
    }
    if (frame_header.size < 16 || frame_header.size > static_cast<unsigned long>(file_end - frame_pos))
    {
      fop->setError("Invalid ASE frame size in frame %d\n", (int)frame);
      break;
    }
    const long frame_end = frame_pos + frame_header.size;

    // Correct frame type
    if (frame_header.magic == ASE_FILE_FRAME_MAGIC)
    {
      // Use frame-duration field?
      if (frame_header.duration > 0)
        sprite->setFrameDuration(frame, frame_header.duration);

      // Read chunks. Every chunk takes at least 6 bytes of the frame, so a
      // bogus count can't run past the frame's data.
      for (uint32_t c = 0; c < frame_header.chunks; c++)
      {
        if (fop->isStop())
          break;
        const long chunk_pos = ftell(f);
        if (chunk_pos < 0 || chunk_pos > frame_end || frame_end - chunk_pos < 6 || feof(f) || ferror(f))
        {
          fop->setError("Truncated ASE chunk header in frame %d\n", (int)frame);
          break;
        }
        fop->setProgress((float)chunk_pos / (float)header.size);

        // Read chunk information
        const uint32_t chunk_size = fgetl(f);
        const int chunk_type = fgetw(f);
        if (feof(f) || ferror(f) || chunk_size < 6 || chunk_size > static_cast<unsigned long>(frame_end - chunk_pos))
        {
          fop->setError("Invalid ASE chunk size in frame %d\n", (int)frame);
          break;
        }
        const long chunk_end = chunk_pos + chunk_size;

        AseReader reader(f, chunk_size - 6);
        try
        {
          switch (chunk_type)
          {

          case ASE_FILE_CHUNK_FLI_COLOR:
          case ASE_FILE_CHUNK_FLI_COLOR2:
            if (!ignore_old_color_chunks)
            {
              Palette* prevPal = sprite->palette(frame);
              auto pal = chunk_type == ASE_FILE_CHUNK_FLI_COLOR ? ase_file_read_color_chunk(reader, *prevPal, frame)
                                                                : ase_file_read_color2_chunk(reader, *prevPal, frame);
              if (prevPal->countDiff(*pal, nullptr, nullptr) > 0)
                sprite->setPalette(*pal, true);
            }
            break;

          case ASE_FILE_CHUNK_PALETTE:
          {
            Palette* prevPal = sprite->palette(frame);
            auto pal = ase_file_read_palette_chunk(reader, *prevPal, frame);

            if (prevPal->countDiff(*pal, nullptr, nullptr, true) > 0)
              sprite->setPalette(*pal, true);

            ignore_old_color_chunks = true;
            break;
          }

          case ASE_FILE_CHUNK_LAYER:
          {
            last_object_with_user_data =
                ase_file_read_layer_chunk(reader, &header, sprite.get(), &last_layer, &current_level);
            break;
          }

          case ASE_FILE_CHUNK_CEL:
          {
            Cel* cel = ase_file_read_cel_chunk(reader, sprite.get(), frame, sprite->pixelFormat(), fop, &header);
            if (cel)
            {
              last_object_with_user_data = cel->data();
            }
            break;
          }

          case ASE_FILE_CHUNK_MASK:
          {
            Mask* mask = ase_file_read_mask_chunk(reader);
            if (mask)
              delete mask; // TODO add the mask in some place?
            else
              fop->setError("Warning: error loading a mask chunk\n");
            break;
          }

          case ASE_FILE_CHUNK_PATH:
            // Ignore
            break;

          case ASE_FILE_CHUNK_TAGS:
            ase_file_read_frame_tags_chunk(reader, &sprite->frameTags());
            break;

          case ASE_FILE_CHUNK_USER_DATA:
          {
            UserData userData;
            ase_file_read_user_data_chunk(reader, &userData);
            if (last_object_with_user_data)
              last_object_with_user_data->setUserData(userData);
            break;
          }

          case ASE_FILE_CHUNK_COLOR_PROFILE:
          {
            (void)reader.word();  //  type
            (void)reader.word();  //  flags
            (void)reader.dword(); //  gamma
            reader.skip(8);
            break;
          }

          default:
            if (!warned_unknown_chunk)
            {
              fop->setError("Warning: Unsupported chunk type %d (skipping; further warnings suppressed)\n", chunk_type);
              warned_unknown_chunk = true;
            }
            break;
          }
        }
        // A bad chunk only loses itself; the chunk size still locates the next one.
        catch (const std::exception& e)
        {
          if (fop->isStop())
            break;
          fop->setError("Warning: error reading chunk in frame %d: %s\n", (int)frame, e.what());
        }

        if (fseek(f, chunk_end, SEEK_SET) != 0)
        {
          fop->setError("Error seeking to ASE chunk end in frame %d\n", (int)frame);
          break;
        }
      }
    }

    // Skip frame size
    if (fseek(f, frame_end, SEEK_SET) != 0)
    {
      fop->setError("Error seeking to ASE frame end in frame %d\n", (int)frame);
      break;
    }

    // Just one frame?
    if (fop->isOneFrame())
      break;

    if (fop->isStop())
      break;
  }

  fop->createDocument(sprite.get());
  // cppcheck-suppress ignoredReturnValue ; releasing ownership is the
  // point, sprite is now owned by the Document created above.
  sprite.release();

  if (ferror(f))
  {
    fop->setError("Error reading file.\n");
    return false;
  }
  else
  {
    return true;
  }
}

bool AseFormat::onPostLoad(FileOp* fop)
{
  LayerFolder* folder = fop->document()->sprite()->folder();

  // Besprited does not support layer groups yet: flatten any group
  // layers into top-level layers so the rest of the app (which assumes
  // a flat layer list, e.g. the timeline) keeps working.
  if (ase_has_groups(folder))
  {
    if (fop->context() && fop->context()->isUIAvailable() &&
        ui::Alert::show("Warning"
                        "<<The file \"%s\" contains layer groups, which %s does not support yet."
                        "<<Open it anyway? Layers inside groups will become top-level layers"
                        "<<named \"Group-Layer\", and the groups will be lost if you save the file."
                        "||&Open||&Cancel",
                        base::get_file_name(fop->filename()).c_str(), PACKAGE) != 1)
    {
      return false;
    }
    ase_ungroup_all(folder);
  }
  return true;
}

bool AseFormat::onSave(FileOp* fop)
{
  const Sprite* sprite = fop->document()->sprite();
  FileHandle handle(open_file_with_exception(fop->filename(), "wb"));
  FILE* f = handle.get();

  // Write the header
  ASE_Header header;
  ase_file_prepare_header(f, &header, sprite);
  ase_file_write_header(f, &header);

  bool require_new_palette_chunk = false;
  for (auto& pal : sprite->getPalettes())
  {
    if (pal->size() != 256 || pal->hasAlpha() || pal->hasNames())
    {
      require_new_palette_chunk = true;
      break;
    }
  }

  // Write frames
  for (frame_t frame(0); frame < sprite->totalFrames(); ++frame)
  {
    // Prepare the frame header
    ASE_FrameHeader frame_header;
    ase_file_prepare_frame_header(f, &frame_header);

    // Frame duration
    frame_header.duration = sprite->frameDuration(frame);

    // is the first frame or did the palette change?
    auto pal = sprite->palette(frame);
    int palFrom = 0, palTo = pal->size() - 1;
    if ((frame == 0 || sprite->palette(frame - 1)->countDiff(*pal, &palFrom, &palTo, true) > 0))
    {
      // Write new palette chunk
      if (require_new_palette_chunk)
      {
        ase_file_write_palette_chunk(f, &frame_header, pal, palFrom, palTo);
      }

      // Write color chunk for backward compatibility only
      ase_file_write_color2_chunk(f, &frame_header, pal);
    }

    // Write extra chunks in the first frame
    if (frame == 0)
    {
      LayerIterator it = sprite->folder()->getLayerBegin();
      LayerIterator end = sprite->folder()->getLayerEnd();

      // Write layer chunks
      for (; it != end; ++it)
        ase_file_write_layers(f, &frame_header, *it);

      // Writer frame tags
      if (sprite->frameTags().size() > 0)
        ase_file_write_frame_tags_chunk(f, &frame_header, &sprite->frameTags());
    }

    // Write cel chunks
    ase_file_write_cels(f, &frame_header, sprite, sprite->folder(), frame);

    // Write the frame header
    ase_file_write_frame_header(f, &frame_header);

    // Progress
    if (sprite->totalFrames() > 1)
      fop->setProgress(float(frame + 1) / float(sprite->totalFrames()));

    if (fop->isStop())
      break;
  }

  // Write the missing field (filesize) of the header.
  ase_file_write_header_filesize(f, &header);

  if (ferror(f))
  {
    fop->setError("Error writing file.\n");
    return false;
  }
  else
  {
    return true;
  }
}

static bool ase_file_read_header(FILE* f, ASE_Header* header)
{
  header->pos = ftell(f);

  header->size = fgetl(f);
  header->magic = fgetw(f);
  if (header->magic != ASE_FILE_MAGIC)
    return false;

  header->frames = fgetw(f);
  header->width = fgetw(f);
  header->height = fgetw(f);
  header->depth = fgetw(f);
  header->flags = fgetl(f);
  header->speed = fgetw(f);
  header->next = fgetl(f);
  header->frit = fgetl(f);
  header->transparent_index = fgetc(f);
  header->ignore[0] = fgetc(f);
  header->ignore[1] = fgetc(f);
  header->ignore[2] = fgetc(f);
  header->ncolors = fgetw(f);
  if (header->ncolors == 0) // 0 means 256 (old .ase files)
    header->ncolors = 256;

  fseek(f, header->pos + 128, SEEK_SET);
  return true;
}

static void ase_file_prepare_header(FILE* f, ASE_Header* header, const Sprite* sprite)
{
  header->pos = ftell(f);

  header->size = 0;
  header->magic = ASE_FILE_MAGIC;
  header->frames = sprite->totalFrames();
  header->width = sprite->width();
  header->height = sprite->height();
  header->depth = (sprite->pixelFormat() == IMAGE_RGB         ? 32
                   : sprite->pixelFormat() == IMAGE_GRAYSCALE ? 16
                   : sprite->pixelFormat() == IMAGE_INDEXED   ? 8
                                                              : 0);
  header->flags = ASE_FILE_FLAG_LAYER_WITH_OPACITY;
  header->speed = sprite->frameDuration(frame_t(0));
  header->next = 0;
  header->frit = 0;
  header->transparent_index = sprite->transparentColor();
  header->ignore[0] = 0;
  header->ignore[1] = 0;
  header->ignore[2] = 0;
  header->ncolors = sprite->palette(frame_t(0))->size();
}

static void ase_file_write_header(FILE* f, ASE_Header* header)
{
  fseek(f, header->pos, SEEK_SET);

  fputl(header->size, f);
  fputw(header->magic, f);
  fputw(header->frames, f);
  fputw(header->width, f);
  fputw(header->height, f);
  fputw(header->depth, f);
  fputl(header->flags, f);
  fputw(header->speed, f);
  fputl(header->next, f);
  fputl(header->frit, f);
  fputc(header->transparent_index, f);
  fputc(header->ignore[0], f);
  fputc(header->ignore[1], f);
  fputc(header->ignore[2], f);
  fputw(header->ncolors, f);

  fseek(f, header->pos + 128, SEEK_SET);
}

static void ase_file_write_header_filesize(FILE* f, ASE_Header* header)
{
  header->size = ftell(f) - header->pos;

  fseek(f, header->pos, SEEK_SET);
  fputl(header->size, f);

  fseek(f, header->pos + header->size, SEEK_SET);
}

static bool ase_file_read_frame_header(FILE* f, ASE_FrameHeader* frame_header)
{
  uint8_t bytes[16];
  if (fread(bytes, 1, sizeof(bytes), f) != sizeof(bytes))
    return false;
  const auto word = [&bytes](int offset) -> uint16_t
  { return uint16_t(bytes[offset]) | (uint16_t(bytes[offset + 1]) << 8); };
  const auto dword = [&word](int offset) -> uint32_t
  { return uint32_t(word(offset)) | (uint32_t(word(offset + 2)) << 16); };
  frame_header->size = dword(0);
  frame_header->magic = word(4);
  frame_header->chunks = word(6);
  frame_header->duration = word(8);
  const uint32_t new_chunks = dword(12);
  if (new_chunks != 0)
    frame_header->chunks = new_chunks;
  return true;
}

static void ase_file_prepare_frame_header(FILE* f, ASE_FrameHeader* frame_header)
{
  int pos = ftell(f);

  frame_header->size = pos;
  frame_header->magic = ASE_FILE_FRAME_MAGIC;
  frame_header->chunks = 0;
  frame_header->duration = 0;

  fseek(f, pos + 16, SEEK_SET);
}

static void ase_file_write_frame_header(FILE* f, ASE_FrameHeader* frame_header)
{
  int pos = frame_header->size;
  int end = ftell(f);

  frame_header->size = end - pos;

  fseek(f, pos, SEEK_SET);

  fputl(frame_header->size, f);
  fputw(frame_header->magic, f);
  fputw(frame_header->chunks < 0xffff ? frame_header->chunks : 0xffff, f);
  fputw(frame_header->duration, f);
  ase_file_write_padding(f, 2);
  fputl(frame_header->chunks < 0xffff ? 0 : frame_header->chunks, f);

  fseek(f, end, SEEK_SET);
}

static void ase_file_write_layers(FILE* f, ASE_FrameHeader* frame_header, const Layer* layer)
{
  ase_file_write_layer_chunk(f, frame_header, layer);
  if (!layer->userData().isEmpty())
    ase_file_write_user_data_chunk(f, frame_header, &layer->userData());

  if (layer->isFolder())
  {
    auto it = static_cast<const LayerFolder*>(layer)->getLayerBegin(),
         end = static_cast<const LayerFolder*>(layer)->getLayerEnd();

    for (; it != end; ++it)
      ase_file_write_layers(f, frame_header, *it);
  }
}

static void ase_file_write_cels(FILE* f, ASE_FrameHeader* frame_header, const Sprite* sprite, const Layer* layer,
                                frame_t frame)
{
  if (layer->isImage())
  {
    if (auto cel = layer->cel(frame))
    {
      /*       fop->setError("New cel in frame %d, in layer %d\n", */
      /*                   frame, sprite_layer2index(sprite, layer)); */

      ase_file_write_cel_chunk(f, frame_header, cel.get(), static_cast<const LayerImage*>(layer), sprite);

      if (!cel->link() && !cel->data()->userData().isEmpty())
      {
        ase_file_write_user_data_chunk(f, frame_header, &cel->data()->userData());
      }
    }
  }

  if (layer->isFolder())
  {
    auto it = static_cast<const LayerFolder*>(layer)->getLayerBegin(),
         end = static_cast<const LayerFolder*>(layer)->getLayerEnd();

    for (; it != end; ++it)
      ase_file_write_cels(f, frame_header, sprite, *it, frame);
  }
}

static void ase_file_write_padding(FILE* f, int bytes)
{
  for (int c = 0; c < bytes; c++)
    fputc(0, f);
}

static void ase_file_write_string(FILE* f, const std::string& string)
{
  fputw(string.size(), f);

  for (size_t c = 0; c < string.size(); ++c)
    fputc(string[c], f);
}

static void ase_file_write_start_chunk(FILE* f, ASE_FrameHeader* frame_header, int type, ASE_Chunk* chunk)
{
  frame_header->chunks++;

  chunk->type = type;
  chunk->start = ftell(f);

  fseek(f, chunk->start + 6, SEEK_SET);
}

static void ase_file_write_close_chunk(FILE* f, ASE_Chunk* chunk)
{
  int chunk_end = ftell(f);
  int chunk_size = chunk_end - chunk->start;

  fseek(f, chunk->start, SEEK_SET);
  fputl(chunk_size, f);
  fputw(chunk->type, f);
  fseek(f, chunk_end, SEEK_SET);
}

static std::shared_ptr<Palette> ase_file_read_color_chunk(AseReader& f, const Palette& prevPal, frame_t frame)
{
  int i, c, r, g, b, packets, skip, size;
  auto pal = prevPal.clone();
  pal->setFrame(frame);

  packets = f.word(); // Number of packets
  skip = 0;

  // Read all packets
  for (i = 0; i < packets; i++)
  {
    skip += f.byte();
    size = f.byte();
    if (!size)
      size = 256;

    for (c = skip; c < skip + size; c++)
    {
      r = std::min<int>(f.byte(), 63);
      g = std::min<int>(f.byte(), 63);
      b = std::min<int>(f.byte(), 63);
      pal->setEntry(c, rgba(scale_6bits_to_8bits(r), scale_6bits_to_8bits(g), scale_6bits_to_8bits(b), 255));
    }
  }

  return pal;
}

static std::shared_ptr<Palette> ase_file_read_color2_chunk(AseReader& f, const Palette& prevPal, frame_t frame)
{
  int i, c, r, g, b, packets, skip, size;
  auto pal = prevPal.clone();
  pal->setFrame(frame);

  packets = f.word(); // Number of packets
  skip = 0;

  // Read all packets
  for (i = 0; i < packets; i++)
  {
    skip += f.byte();
    size = f.byte();
    if (!size)
      size = 256;

    for (c = skip; c < skip + size; c++)
    {
      r = f.byte();
      g = f.byte();
      b = f.byte();
      pal->setEntry(c, rgba(r, g, b, 255));
    }
  }

  return pal;
}

static std::shared_ptr<Palette> ase_file_read_palette_chunk(AseReader& f, const Palette& prevPal, frame_t frame)
{
  auto pal = prevPal.clone();
  pal->setFrame(frame);

  int newSize = f.dword();
  int from = f.dword();
  int to = f.dword();
  f.skip(8);

  // newSize/from/to are raw 32-bit fields with no format-level upper bound.
  // Palette::setEntry() below is bounds-checked so this loop can't corrupt
  // memory either way, but an oversized newSize is still a multi-GB
  // resize() attempt, and an oversized `to` is a CPU-exhaustion DoS
  // (reading past real EOF for up to ~2 billion iterations). Cap both to a
  // sane ceiling (see issue #219).
  constexpr int kMaxPaletteSize = 65536;
  if (newSize < 0 || newSize > kMaxPaletteSize)
    newSize = 0;
  if (newSize > 0)
    pal->resize(newSize);

  if (from < 0)
    from = 0;
  if (to > kMaxPaletteSize - 1)
    to = kMaxPaletteSize - 1;

  for (int c = from; c <= to; ++c)
  {
    int flags = f.word();
    int r = f.byte();
    int g = f.byte();
    int b = f.byte();
    int a = f.byte();
    pal->setEntry(c, rgba(r, g, b, a));

    std::string name;
    if (flags & ASE_PALETTE_FLAG_HAS_NAME)
      name = f.string();
    pal->setEntryName(c, name);
  }

  return pal;
}

static void ase_file_write_color2_chunk(FILE* f, ASE_FrameHeader* frame_header, const Palette* pal)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_FLI_COLOR2);
  int c, color;

  fputw(1, f); // Number of packets

  // First packet
  fputc(0, f);                                    // skip 0 colors
  fputc(pal->size() == 256 ? 0 : pal->size(), f); // number of colors
  for (c = 0; c < pal->size(); c++)
  {
    color = pal->getEntry(c);
    fputc(rgba_getr(color), f);
    fputc(rgba_getg(color), f);
    fputc(rgba_getb(color), f);
  }
}

static void ase_file_write_palette_chunk(FILE* f, ASE_FrameHeader* frame_header, const Palette* pal, int from, int to)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_PALETTE);

  fputl(pal->size(), f);
  fputl(from, f);
  fputl(to, f);
  ase_file_write_padding(f, 8);

  for (int c = from; c <= to; ++c)
  {
    color_t color = pal->getEntry(c);
    const std::string& name = pal->entryName(c);
    fputw(name.empty() ? 0 : ASE_PALETTE_FLAG_HAS_NAME, f);
    fputc(rgba_getr(color), f);
    fputc(rgba_getg(color), f);
    fputc(rgba_getb(color), f);
    fputc(rgba_geta(color), f);
    if (!name.empty())
      ase_file_write_string(f, name);
  }
}

static Layer* ase_file_read_layer_chunk(AseReader& f, ASE_Header* header, Sprite* sprite, Layer** previous_layer,
                                        int* current_level)
{
  std::string name;
  std::unique_ptr<Layer> layer;
  /* read chunk data */
  int flags;
  int layer_type;
  int child_level;

  flags = f.word();
  layer_type = f.word();
  child_level = f.word();
  f.word();                 // default width
  f.word();                 // default height
  int blendmode = f.word(); // blend mode
  int opacity = f.byte();   // opacity

  f.skip(3);
  name = f.string();

  // Image layer
  if (layer_type == 0)
  {
    layer = std::make_unique<LayerImage>(sprite);

    // Only transparent layers can have blend mode and opacity
    if (!(flags & ASE_LAYER_FLAG_BACKGROUND))
    {
      static_cast<LayerImage*>(layer.get())->setBlendMode((BlendMode)blendmode);
      if (header->flags & ASE_FILE_FLAG_LAYER_WITH_OPACITY)
        static_cast<LayerImage*>(layer.get())->setOpacity(opacity);
    }
  }
  // Layer set
  else if (layer_type == 1)
  {
    layer = std::make_unique<LayerFolder>(sprite);
  }

  if (layer)
  {
    // flags
    layer->setFlags(static_cast<LayerFlags>(flags));

    // name
    layer->setName(name.c_str());

    // Child level
    LayerFolder* parent = nullptr;
    if (child_level == *current_level)
      parent = (*previous_layer)->parent();
    else if (child_level > *current_level && (*previous_layer)->isFolder())
      parent = static_cast<LayerFolder*>(*previous_layer);
    else if (child_level < *current_level)
    {
      // Ascend as many levels as the child_level dropped (a drop can
      // skip several nested groups at once, not just one level).
      parent = (*previous_layer)->parent();
      for (int lvl = *current_level; lvl > child_level && parent; --lvl)
        parent = parent->parent();
    }

    if (!parent)
    {
      // Corrupt/unexpected child level sequence: fall back to the root
      // folder instead of crashing on a NULL/invalid parent.
      parent = sprite->folder();
    }

    parent->addLayer(layer.get());

    *previous_layer = layer.get();
    *current_level = child_level;
  }

  return layer.release();
}

static void ase_file_write_layer_chunk(FILE* f, ASE_FrameHeader* frame_header, const Layer* layer)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_LAYER);

  // Flags
  fputw(static_cast<int>(layer->flags()), f);

  // Layer type
  fputw(layer->isImage() ? 0 : (layer->isFolder() ? 1 : -1), f);

  // Layer child level
  LayerFolder* parent = layer->parent();
  int child_level = -1;
  while (parent != nullptr)
  {
    child_level++;
    parent = parent->parent();
  }
  fputw(child_level, f);

  // Default width & height, and blend mode
  fputw(0, f);
  fputw(0, f);
  fputw(layer->isImage() ? (int)static_cast<const LayerImage*>(layer)->blendMode() : 0, f);
  fputc(layer->isImage() ? (int)static_cast<const LayerImage*>(layer)->opacity() : 0, f);

  // padding
  ase_file_write_padding(f, 3);

  /* layer name */
  ase_file_write_string(f, layer->name());

  /* fop->setError("Layer name \"%s\" child level: %d\n", layer->name,
   * child_level); */
}

//////////////////////////////////////////////////////////////////////
// Pixel I/O
//////////////////////////////////////////////////////////////////////

template <typename ImageTraits> class PixelIO
{
public:
  typename ImageTraits::pixel_t read_pixel(AseReader& f);
  void write_pixel(FILE* f, typename ImageTraits::pixel_t c);
  void read_scanline(typename ImageTraits::address_t address, int w, uint8_t* buffer);
  void write_scanline(typename ImageTraits::address_t address, int w, uint8_t* buffer);
};

template <> class PixelIO<RgbTraits>
{
  int r, g, b, a;

public:
  RgbTraits::pixel_t read_pixel(AseReader& f)
  {
    r = f.byte();
    g = f.byte();
    b = f.byte();
    a = f.byte();
    return rgba(r, g, b, a);
  }
  void write_pixel(FILE* f, RgbTraits::pixel_t c)
  {
    fputc(rgba_getr(c), f);
    fputc(rgba_getg(c), f);
    fputc(rgba_getb(c), f);
    fputc(rgba_geta(c), f);
  }
  void read_scanline(RgbTraits::address_t address, int w, uint8_t* buffer)
  {
    for (int x = 0; x < w; ++x)
    {
      r = *(buffer++);
      g = *(buffer++);
      b = *(buffer++);
      a = *(buffer++);
      *(address++) = rgba(r, g, b, a);
    }
  }
  void write_scanline(RgbTraits::address_t address, int w, uint8_t* buffer)
  {
    for (int x = 0; x < w; ++x)
    {
      *(buffer++) = rgba_getr(*address);
      *(buffer++) = rgba_getg(*address);
      *(buffer++) = rgba_getb(*address);
      *(buffer++) = rgba_geta(*address);
      ++address;
    }
  }
};

template <> class PixelIO<GrayscaleTraits>
{
  int k, a;

public:
  GrayscaleTraits::pixel_t read_pixel(AseReader& f)
  {
    k = f.byte();
    a = f.byte();
    return graya(k, a);
  }
  void write_pixel(FILE* f, GrayscaleTraits::pixel_t c)
  {
    fputc(graya_getv(c), f);
    fputc(graya_geta(c), f);
  }
  void read_scanline(GrayscaleTraits::address_t address, int w, uint8_t* buffer)
  {
    for (int x = 0; x < w; ++x)
    {
      k = *(buffer++);
      a = *(buffer++);
      *(address++) = graya(k, a);
    }
  }
  void write_scanline(GrayscaleTraits::address_t address, int w, uint8_t* buffer)
  {
    for (int x = 0; x < w; ++x)
    {
      *(buffer++) = graya_getv(*address);
      *(buffer++) = graya_geta(*address);
      ++address;
    }
  }
};

template <> class PixelIO<IndexedTraits>
{
public:
  IndexedTraits::pixel_t read_pixel(AseReader& f) { return f.byte(); }
  void write_pixel(FILE* f, IndexedTraits::pixel_t c) { fputc(c, f); }
  void read_scanline(IndexedTraits::address_t address, int w, uint8_t* buffer) { memcpy(address, buffer, w); }
  void write_scanline(IndexedTraits::address_t address, int w, uint8_t* buffer) { memcpy(buffer, address, w); }
};

//////////////////////////////////////////////////////////////////////
// Raw Image
//////////////////////////////////////////////////////////////////////

template <typename ImageTraits> static void read_raw_image(AseReader& f, Image* image, FileOp* fop, ASE_Header* header)
{
  PixelIO<ImageTraits> pixel_io;
  int x, y;

  for (y = 0; y < image->height(); y++)
  {
    if ((y & 63) == 0)
    {
      if (fop->isStop())
        throw base::Exception("ASE loading cancelled.");
      fop->setProgress((float)f.position() / (float)header->size);
    }
    for (x = 0; x < image->width(); x++)
      put_pixel_fast<ImageTraits>(image, x, y, pixel_io.read_pixel(f));
  }
}

template <typename ImageTraits> static void write_raw_image(FILE* f, const Image* image)
{
  PixelIO<ImageTraits> pixel_io;
  int x, y;

  for (y = 0; y < image->height(); y++)
    for (x = 0; x < image->width(); x++)
      pixel_io.write_pixel(f, get_pixel_fast<ImageTraits>(image, x, y));
}

//////////////////////////////////////////////////////////////////////
// Compressed Image
//////////////////////////////////////////////////////////////////////

template <typename ImageTraits>
static void read_compressed_image(AseReader& f, Image* image, FileOp* fop, ASE_Header* header)
{
  z_stream stream{};
  if (inflateInit(&stream) != Z_OK)
    throw base::Exception("Cannot initialize ASE image decompression.");
  struct InflateGuard
  {
    z_stream* stream;
    ~InflateGuard() { inflateEnd(stream); }
  } guard{&stream};

  PixelIO<ImageTraits> pixel_io;
  std::array<uint8_t, 4096> input;
  std::vector<uint8_t> row(ImageTraits::getRowStrideBytes(image->width()));
  size_t row_offset = 0;
  // Stop once every row is decoded: bytes past the image (including a
  // missing or damaged zlib trailer) are skipped with the rest of the chunk.
  for (int y = 0; y < image->height();)
  {
    if (stream.avail_in == 0)
    {
      if (f.remaining() == 0)
        throw base::Exception("Incomplete compressed ASE image.");
      if (fop->isStop())
        throw base::Exception("ASE loading cancelled.");
      fop->setProgress((float)f.position() / (float)header->size);

      const size_t count = std::min(f.remaining(), input.size());
      f.read(input.data(), count);
      stream.next_in = input.data();
      stream.avail_in = static_cast<uInt>(count);
    }

    stream.next_out = row.data() + row_offset;
    stream.avail_out = static_cast<uInt>(row.size() - row_offset);
    const uInt before_in = stream.avail_in;
    const int result = inflate(&stream, Z_NO_FLUSH);
    if (result != Z_OK && result != Z_STREAM_END && result != Z_BUF_ERROR)
      throw base::Exception("Invalid compressed ASE image.");
    const size_t produced = row.size() - row_offset - stream.avail_out;
    row_offset += produced;
    if (row_offset == row.size())
    {
      auto address = reinterpret_cast<typename ImageTraits::address_t>(image->getPixelAddress(0, y));
      pixel_io.read_scanline(address, image->width(), row.data());
      ++y;
      row_offset = 0;
    }
    else if (result == Z_STREAM_END || (before_in == stream.avail_in && produced == 0))
      throw base::Exception("Compressed ASE image has missing pixels.");
  }
}

template <typename ImageTraits> static void write_compressed_image(FILE* f, const Image* image)
{
  PixelIO<ImageTraits> pixel_io;
  z_stream zstream;
  int y, err;

  zstream.zalloc = (alloc_func)0;
  zstream.zfree = (free_func)0;
  zstream.opaque = (voidpf)0;
  err = deflateInit(&zstream, Z_DEFAULT_COMPRESSION);
  if (err != Z_OK)
    throw base::Exception("ZLib error %d in deflateInit().", err);

  std::vector<uint8_t> scanline(ImageTraits::getRowStrideBytes(image->width()));
  std::vector<uint8_t> compressed(4096);

  for (y = 0; y < image->height(); y++)
  {
    typename ImageTraits::address_t address = (typename ImageTraits::address_t)image->getPixelAddress(0, y);

    pixel_io.write_scanline(address, image->width(), &scanline[0]);

    zstream.next_in = reinterpret_cast<Bytef*>(&scanline[0]);
    zstream.avail_in = scanline.size();
    int flush = (y == image->height() - 1 ? Z_FINISH : Z_NO_FLUSH);

    do
    {
      zstream.next_out = reinterpret_cast<Bytef*>(&compressed[0]);
      zstream.avail_out = compressed.size();

      // Compress
      err = deflate(&zstream, flush);
      if (err != Z_OK && err != Z_STREAM_END && err != Z_BUF_ERROR)
        throw base::Exception("ZLib error %d in deflate().", err);

      int output_bytes = compressed.size() - zstream.avail_out;
      if (output_bytes > 0)
      {
        if ((fwrite(&compressed[0], 1, output_bytes, f) != (size_t)output_bytes) || ferror(f))
          throw base::Exception("Error writing compressed image pixels.\n");
      }
    } while (zstream.avail_out == 0);
  }

  err = deflateEnd(&zstream);
  if (err != Z_OK)
    throw base::Exception("ZLib error %d in deflateEnd().", err);
}

//////////////////////////////////////////////////////////////////////
// Cel Chunk
//////////////////////////////////////////////////////////////////////

static Cel* ase_file_read_cel_chunk(AseReader& f, Sprite* sprite, frame_t frame, PixelFormat pixelFormat, FileOp* fop,
                                    ASE_Header* header)
{
  /* read chunk data */
  LayerIndex layer_index = LayerIndex(f.word());
  int x = ((short)f.word());
  int y = ((short)f.word());
  int opacity = f.byte();
  int cel_type = f.word();
  Layer* layer;

  f.skip(7);

  layer = sprite->indexToLayer(layer_index);
  if (!layer)
  {
    fop->setError("Frame %d didn't found layer with index %d\n", (int)frame, (int)layer_index);
    return nullptr;
  }
  if (!layer->isImage())
  {
    fop->setError("Invalid .ase file (frame %d in layer %d which does not "
                  "contain images\n",
                  (int)frame, (int)layer_index);
    return nullptr;
  }

  // Create the new frame.
  std::shared_ptr<Cel> cel;

  switch (cel_type)
  {

  case ASE_FILE_RAW_CEL:
  {
    // Read width and height
    int w = f.word();
    int h = f.word();

    if (w > 0 && h > 0 && w <= kMaxFileImageDimension && h <= kMaxFileImageDimension)
    {
      const size_t bytes_per_pixel = pixelFormat == IMAGE_RGB ? 4 : (pixelFormat == IMAGE_GRAYSCALE ? 2 : 1);
      f.require(size_t(w) * size_t(h) * bytes_per_pixel);
      ImageRef image(Image::create(pixelFormat, w, h));

      // Read pixel data
      switch (image->pixelFormat())
      {

      case IMAGE_RGB:
        read_raw_image<RgbTraits>(f, image.get(), fop, header);
        break;

      case IMAGE_GRAYSCALE:
        read_raw_image<GrayscaleTraits>(f, image.get(), fop, header);
        break;

      case IMAGE_INDEXED:
        read_raw_image<IndexedTraits>(f, image.get(), fop, header);
        break;
      }

      cel = std::make_shared<Cel>(frame, image);
      cel->setPosition(x, y);
      cel->setOpacity(opacity);
    }
    break;
  }

  case ASE_FILE_LINK_CEL:
  {
    // Read link position
    frame_t link_frame = frame_t(f.word());
    if (auto link = layer->cel(link_frame))
    {
      // There were a beta version that allow to the user specify
      // different X, Y, or opacity per link, in that case we must
      // create a copy.
      if (link->x() == x && link->y() == y && link->opacity() == opacity)
      {
        cel = Cel::createLink(link);
        cel->setFrame(frame);
      }
      else
      {
        cel = Cel::createCopy(link);
        cel->setFrame(frame);
        cel->setPosition(x, y);
        cel->setOpacity(opacity);
      }
    }
    else
    {
      // Linked cel not found
      return nullptr;
    }
    break;
  }

  case ASE_FILE_COMPRESSED_CEL:
  {
    // Read width and height
    int w = f.word();
    int h = f.word();

    if (w > 0 && h > 0 && w <= kMaxFileImageDimension && h <= kMaxFileImageDimension)
    {
      f.require(2); // A zlib stream must at least contain its header.
      ImageRef image(Image::create(pixelFormat, w, h));

      // A damaged stream keeps the rows decoded so far (the rest stay clear)
      // and doesn't stop loading the other cels.
      clear_image(image.get(), 0);
      try
      {
        switch (image->pixelFormat())
        {

        case IMAGE_RGB:
          read_compressed_image<RgbTraits>(f, image.get(), fop, header);
          break;

        case IMAGE_GRAYSCALE:
          read_compressed_image<GrayscaleTraits>(f, image.get(), fop, header);
          break;

        case IMAGE_INDEXED:
          read_compressed_image<IndexedTraits>(f, image.get(), fop, header);
          break;
        }
      }
      catch (const std::exception& e)
      {
        if (fop->isStop())
          throw;
        fop->setError("Warning: %s\n", e.what());
      }

      cel = std::make_shared<Cel>(frame, image);
      cel->setPosition(x, y);
      cel->setOpacity(opacity);
    }
    break;
  }
  }

  if (cel)
    static_cast<LayerImage*>(layer)->addCel(cel);

  return cel.get();
}

static void ase_file_write_cel_chunk(FILE* f, ASE_FrameHeader* frame_header, const Cel* cel, const LayerImage* layer,
                                     const Sprite* sprite)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_CEL);

  int layer_index = sprite->layerToIndex(layer);
  auto link = cel->link();
  int cel_type = (link ? ASE_FILE_LINK_CEL : ASE_FILE_COMPRESSED_CEL);

  fputw(layer_index, f);
  fputw(cel->x(), f);
  fputw(cel->y(), f);
  fputc(cel->opacity(), f);
  fputw(cel_type, f);
  ase_file_write_padding(f, 7);

  switch (cel_type)
  {

  case ASE_FILE_RAW_CEL:
  {
    const Image* image = cel->image();

    if (image)
    {
      // Width and height
      fputw(image->width(), f);
      fputw(image->height(), f);

      // Pixel data
      switch (image->pixelFormat())
      {

      case IMAGE_RGB:
        write_raw_image<RgbTraits>(f, image);
        break;

      case IMAGE_GRAYSCALE:
        write_raw_image<GrayscaleTraits>(f, image);
        break;

      case IMAGE_INDEXED:
        write_raw_image<IndexedTraits>(f, image);
        break;
      }
    }
    else
    {
      // Width and height
      fputw(0, f);
      fputw(0, f);
    }
    break;
  }

  case ASE_FILE_LINK_CEL:
    // Linked cel to another frame
    fputw(link->frame(), f);
    break;

  case ASE_FILE_COMPRESSED_CEL:
  {
    const Image* image = cel->image();

    if (image)
    {
      // Width and height
      fputw(image->width(), f);
      fputw(image->height(), f);

      // Pixel data
      switch (image->pixelFormat())
      {

      case IMAGE_RGB:
        write_compressed_image<RgbTraits>(f, image);
        break;

      case IMAGE_GRAYSCALE:
        write_compressed_image<GrayscaleTraits>(f, image);
        break;

      case IMAGE_INDEXED:
        write_compressed_image<IndexedTraits>(f, image);
        break;
      }
    }
    else
    {
      // Width and height
      fputw(0, f);
      fputw(0, f);
    }
    break;
  }
  }
}

static Mask* ase_file_read_mask_chunk(AseReader& f)
{
  int c, u, v, byte;
  // Read chunk data
  int x = f.word();
  int y = f.word();
  int w = f.word();
  int h = f.word();

  f.skip(8);
  std::string name = f.string();

  if (w > kMaxFileImageDimension || h > kMaxFileImageDimension)
    return nullptr;

  f.require(((static_cast<size_t>(w) + 7) / 8) * static_cast<size_t>(h));
  auto mask = std::make_unique<Mask>();
  mask->setName(name.c_str());
  mask->replace(gfx::Rect(x, y, w, h));

  // Read image data
  for (v = 0; v < h; v++)
    for (u = 0; u < (w + 7) / 8; u++)
    {
      byte = f.byte();
      for (c = 0; c < 8 && u * 8 + c < w; c++)
        put_pixel(mask->bitmap(), u * 8 + c, v, byte & (1 << (7 - c)));
    }

  return mask.release();
}

#if 0
static void ase_file_write_mask_chunk(FILE* f, ASE_FrameHeader* frame_header, Mask* mask)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_MASK);

  int c, u, v, byte;
  const gfx::Rect& bounds(mask->bounds());

  fputw(bounds.x, f);
  fputw(bounds.y, f);
  fputw(bounds.w, f);
  fputw(bounds.h, f);
  ase_file_write_padding(f, 8);

  // Name
  ase_file_write_string(f, mask->name().c_str());

  // Bitmap
  for (v=0; v<bounds.h; v++)
    for (u=0; u<(bounds.w+7)/8; u++) {
      byte = 0;
      for (c=0; c<8; c++)
        if (get_pixel(mask->bitmap(), u*8+c, v))
          byte |= (1<<(7-c));
      fputc(byte, f);
    }
}
#endif

static void ase_file_read_frame_tags_chunk(AseReader& f, FrameTags* frameTags)
{
  const unsigned tags = f.word();
  f.skip(8);
  if (tags > f.remaining() / 19)
    throw base::Exception("Invalid ASE frame tag count.");

  for (unsigned c = 0; c < tags; ++c)
  {
    frame_t from = f.word();
    frame_t to = f.word();
    int aniDir = f.byte();
    if (aniDir != int(AniDir::FORWARD) && aniDir != int(AniDir::REVERSE) && aniDir != int(AniDir::PING_PONG))
    {
      aniDir = int(AniDir::FORWARD);
    }

    f.dword(); // 8 reserved bytes
    f.dword();

    int r = f.byte();
    int g = f.byte();
    int b = f.byte();
    f.byte(); // Skip

    const std::string name = f.string();

    auto tag = std::make_unique<FrameTag>(from, to);
    tag->setColor(doc::rgba(r, g, b, 255));
    tag->setName(name);
    tag->setAniDir((AniDir)aniDir);
    frameTags->add(tag.get());
    tag.release();
  }
}

static void ase_file_write_frame_tags_chunk(FILE* f, ASE_FrameHeader* frame_header, const FrameTags* frameTags)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_TAGS);

  fputw(frameTags->size(), f);

  fputl(0, f); // 8 reserved bytes
  fputl(0, f);

  for (const FrameTag* tag : *frameTags)
  {
    fputw(tag->fromFrame(), f);
    fputw(tag->toFrame(), f);
    fputc((int)tag->aniDir(), f);

    fputl(0, f); // 8 reserved bytes
    fputl(0, f);

    fputc(doc::rgba_getr(tag->color()), f);
    fputc(doc::rgba_getg(tag->color()), f);
    fputc(doc::rgba_getb(tag->color()), f);
    fputc(0, f);

    ase_file_write_string(f, tag->name().c_str());
  }
}

static void ase_file_read_user_data_chunk(AseReader& f, UserData* userData)
{
  size_t flags = f.dword();

  if (flags & ASE_USER_DATA_FLAG_HAS_TEXT)
  {
    std::string text = f.string();
    userData->setText(text);
  }

  if (flags & ASE_USER_DATA_FLAG_HAS_COLOR)
  {
    int r = f.byte();
    int g = f.byte();
    int b = f.byte();
    int a = f.byte();
    userData->setColor(doc::rgba(r, g, b, a));
  }
}

static void ase_file_write_user_data_chunk(FILE* f, ASE_FrameHeader* frame_header, const UserData* userData)
{
  ChunkWriter chunk(f, frame_header, ASE_FILE_CHUNK_USER_DATA);

  int flags = 0;
  if (!userData->text().empty())
    flags |= ASE_USER_DATA_FLAG_HAS_TEXT;
  if (doc::rgba_geta(userData->color()))
    flags |= ASE_USER_DATA_FLAG_HAS_COLOR;
  fputl(flags, f);

  if (flags & ASE_USER_DATA_FLAG_HAS_TEXT)
    ase_file_write_string(f, userData->text().c_str());

  if (flags & ASE_USER_DATA_FLAG_HAS_COLOR)
  {
    fputc(doc::rgba_getr(userData->color()), f);
    fputc(doc::rgba_getg(userData->color()), f);
    fputc(doc::rgba_getb(userData->color()), f);
    fputc(doc::rgba_geta(userData->color()), f);
  }
}

static bool ase_has_groups(LayerFolder* folder)
{
  for (Layer* child : folder->getLayersList())
  {
    if (child->isFolder())
      return true;
  }
  return false;
}

// Recursively walks "folder" (in stack order) collecting every image
// layer (renamed to reflect the group chain it was found in) and every
// group folder found underneath it, both in depth-first order.
static void ase_collect_flattened_layers(LayerFolder* folder, LayerFolder* root, LayerList& outImageLayers,
                                         LayerList& outFolders)
{
  for (Layer* child : folder->getLayersList())
  {
    if (child->isFolder())
    {
      ase_collect_flattened_layers(static_cast<LayerFolder*>(child), root, outImageLayers, outFolders);
      outFolders.push_back(child);
    }
    else
    {
      if (folder != root)
      {
        // Create a new name adding all group layer names
        std::string name;
        for (Layer* layer = child; layer != root; layer = layer->parent())
        {
          if (!name.empty())
            name.insert(0, "-");
          name.insert(0, layer->name());
        }
        child->setName(name);
      }
      outImageLayers.push_back(child);
    }
  }
}

// Moves every image layer nested inside a group directly under "root",
// preserving their original relative stacking order, and destroys the
// (now-empty) groups. Unlike a naive "reparent as we go" approach, this
// avoids moving layers to the top of the stack, which would silently
// reorder the visible composition of files with more than one group.
static void ase_ungroup_all(LayerFolder* root)
{
  LayerList imageLayers, folders;
  ase_collect_flattened_layers(root, root, imageLayers, folders);

  // Detach every collected layer/folder from its current parent first,
  // so none of the folders we are about to delete still owns a layer
  // (or nested folder) we need to keep.
  for (Layer* layer : imageLayers)
    layer->parent()->removeLayer(layer);
  for (Layer* folder : folders)
    folder->parent()->removeLayer(folder);

  for (Layer* folder : folders)
  {
    ASSERT(static_cast<LayerFolder*>(folder)->getLayersCount() == 0);
    delete folder;
  }

  // Re-add the image layers to the root, in their original relative
  // stacking order.
  for (Layer* layer : imageLayers)
    root->addLayer(layer);

  ASSERT(!ase_has_groups(root));
}

} // namespace app
