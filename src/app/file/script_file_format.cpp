// LibreSprite | Copyright (C) 2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/file/script_file_format.h"

#include "app/document.h"
#include "app/file/file.h"
#include "app/file/script_codec_requests.h"
#include "app/script/app_scripting.h"
#include "app/task_manager.h"
#include "base/file_handle.h"
#include "doc/cel.h"
#include "doc/color.h"
#include "doc/image.h"
#include "doc/image_ref.h"
#include "doc/layer.h"
#include "doc/palette.h"
#include "doc/sprite.h"
#include "render/render.h"
#include "she/system.h"
#include "ui/manager.h"

#include <chrono>
#include <cstdint>
#include <cstring>
#include <future>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace app
{
namespace
{

// Ceiling for the frame count a codec reply may claim (the same order
// as the FLI/FLC codec): a buggy codec must not be able to claim a
// huge frame count with no data behind it.
constexpr int kMaxScriptCodecFrames = 10000;

// Backstop for the worker-thread future wait: while the main loop
// pumps, the main-thread watchdog is the arbiter; this only covers the
// edge case where it stops pumping (app exit) while a request is still
// in flight. AppScripting::shutdown() fails the request and the FileOp
// stop flag covers the rest; this is the last resort.
constexpr std::chrono::minutes kReplyBackstop{5};

// Descriptor format names. RGB and GRAYA expand to IMAGE_RGB on load
// (doc has no 3-byte RGB or gray+alpha format); GRAY is save-only.
constexpr std::string_view kFormatIndexed = "INDEXED";
constexpr std::string_view kFormatRgb = "RGB";
constexpr std::string_view kFormatRgba = "RGBA";
constexpr std::string_view kFormatGraya = "GRAYA";
constexpr std::string_view kFormatGray = "GRAY";

enum class FrameKind
{
  Indexed,
  Rgb,
  Rgba,
  Graya
};

// A format accepted on load.
struct FrameFormat
{
  FrameKind kind;
  std::string_view name;
  int bytesPerPixel; // size of the data the codec sends
  bool hasAlpha;     // unused for Indexed (decided by transparentIndex)
};

constexpr FrameFormat kLoadFormats[] = {
    {FrameKind::Indexed, kFormatIndexed, 1, false},
    {FrameKind::Rgb, kFormatRgb, 3, false},
    {FrameKind::Rgba, kFormatRgba, 4, true},
    {FrameKind::Graya, kFormatGraya, 2, true},
};

// nullptr if the name is unknown.
const FrameFormat* findLoadFormat(const std::string& name)
{
  for (const auto& f : kLoadFormats)
    if (f.name == name)
      return &f;
  return nullptr;
}

doc::PixelFormat docFormat(const FrameFormat& f)
{
  return f.kind == FrameKind::Indexed ? doc::IMAGE_INDEXED : doc::IMAGE_RGB;
}

// The descriptor format name for a pre-rendered frame (save): the
// sprite's native pixel format (IMAGE_BITMAP expands to RGBA).
std::string_view saveFormatName(doc::PixelFormat format)
{
  switch (format)
  {
  case doc::IMAGE_INDEXED:
    return kFormatIndexed;
  case doc::IMAGE_GRAYSCALE:
    return kFormatGray;
  case doc::IMAGE_RGB:
  case doc::IMAGE_BITMAP:
  default:
    return kFormatRgba;
  }
}

// An integral-range number from a codec reply; false for non-numbers,
// NaN and out-of-range values (so the int cast is well defined).
bool getInt(JSON::Value& v, double lo, double hi, int& out)
{
  if (!v.isNumber())
    return false;
  const double n = v.number();
  if (!(n >= lo && n <= hi))
    return false;
  out = (int)n;
  return true;
}

// A codec reply that failed (the "file" error API or the watchdog).
bool isCodecError(JSON::Value& result)
{
  return result.isObject() && result.object().contains("error");
}

} // namespace

ScriptFileFormat::ScriptFileFormat(const std::string& scriptPath, const std::string& name,
                                   const std::string& extensions, bool load, bool save)
  : m_scriptPath(scriptPath)
  , m_name(name)
  , m_extensions(extensions)
  // The codec receives pre-rendered frames and returns one byte stream
  // per file, so no FILE_SUPPORT_SEQUENCES (the per-frame save protocol
  // does not fit a single-file codec). The pixel-format flags keep the
  // save-warnings path quiet; FILE_SUPPORT_PALETTES keeps the "Palette
  // changes between frames" warning quiet (each frame's palette is
  // passed in the descriptor).
  , m_flags(FILE_SUPPORT_RGB | FILE_SUPPORT_RGBA | FILE_SUPPORT_GRAY | FILE_SUPPORT_GRAYA | FILE_SUPPORT_INDEXED |
            FILE_SUPPORT_FRAMES | FILE_SUPPORT_PALETTES)
{
  if (load)
    m_flags |= FILE_SUPPORT_LOAD;
  if (save)
    m_flags |= FILE_SUPPORT_SAVE;
}

// Raises the codec event and waits for the reply. The reply (the "file"
// API or the watchdog) is delivered on the main thread: on the main
// thread (headless FileOp, or a GUI FileOp that runs there, e.g. the
// exporter) pump the loop until it arrives; on a worker thread (GUI)
// the main thread is already pumping, so just wait.
JSON::Value ScriptFileFormat::dispatchCodec(FileOp* fop, const char* eventName, JSON::Value payload)
{
  auto request = ScriptCodecRequests::create();

  JSON::Value event;
  event.push_back(eventName);
  event.push_back((double)request.id);
  event.push_back(fop->filename());
  event.push_back(std::move(payload));
  AppScripting::raiseEvent(m_scriptPath, event);

  std::string error = "format codec did not reply (timeout)";
  if (she::instance()->isMainThread())
  {
    // Main thread: pump the event loop until the reply (the watchdog in
    // AppScripting::tick() expires a no-reply codec). The same sequence
    // as AppScripting::runHeadlessPump().
    for (;;)
    {
      ui::Manager* mgr = ui::Manager::getDefault();
      if (mgr)
      {
        ui::Timer::pollTimers();
        mgr->generateMessages();
        mgr->pumpQueue();
        mgr->collectGarbage();
      }
      if (TaskManager::exists())
        TaskManager::instance().pump();
      AppScripting::tick();

      if (request.future.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready)
      {
        try
        {
          return request.future.get();
        }
        catch (const std::future_error&)
        {
          break;
        }
      }
    }
  }
  else
  {
    // Worker thread: the main thread is already pumping (the progress
    // window runs a nested message loop, or the main loop): it drains
    // the event and delivers the reply. Also watch the stop flag: once
    // the progress window closes (a user cancel, or the app-exit stop in
    // ~Worker) the main loop stops pumping, so the reply would never be
    // delivered. The backstop covers the rest.
    const auto deadline = std::chrono::steady_clock::now() + kReplyBackstop;
    for (;;)
    {
      if (request.future.wait_for(std::chrono::milliseconds(100)) == std::future_status::ready)
      {
        try
        {
          return request.future.get();
        }
        catch (const std::future_error&)
        {
          break;
        }
      }
      if (fop->isStop())
      {
        error = "operation canceled";
        break;
      }
      if (std::chrono::steady_clock::now() >= deadline)
        break;
    }
  }

  JSON::Value err;
  err.object()["error"] = error;
  return err;
}

bool ScriptFileFormat::onLoad(FileOp* fop)
{
  // Read the file bytes
  std::vector<uint8_t> data;
  {
    base::FileHandle handle(base::open_file_with_exception(fop->filename(), "rb"));
    FILE* f = handle.get();
    if (fseek(f, 0, SEEK_END) != 0)
    {
      fop->setError("Error reading file \"%s\"\n", fop->filename().c_str());
      return false;
    }
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0)
    {
      fop->setError("Error reading file \"%s\"\n", fop->filename().c_str());
      return false;
    }
    data.resize((size_t)size);
    if (size > 0 && std::fread(data.data(), 1, data.size(), f) != data.size())
    {
      fop->setError("Error reading file \"%s\"\n", fop->filename().c_str());
      return false;
    }
  }

  JSON::Value payload;
  payload = JSON::Value{std::make_shared<std::vector<uint8_t>>(std::move(data))};

  JSON::Value result = dispatchCodec(fop, "format_load", std::move(payload));
  if (isCodecError(result))
  {
    fop->setError("Error: %s\n", result["error"].string().c_str());
    return false;
  }

  // The frames: { frames: [descriptor, ...] } or a single descriptor.
  JSON::Array frames;
  if (result.isObject() && result.object().contains("frames"))
  {
    if (!result["frames"].isArray())
    {
      fop->setError("format codec: frames must be an array\n");
      return false;
    }
    frames = result["frames"].array();
  }
  else if (result.isObject())
  {
    frames.push_back(result);
  }
  else
  {
    fop->setError("format codec: expected { frames: [...] }\n");
    return false;
  }
  if (frames.empty())
  {
    fop->setError("format codec: no frames\n");
    return false;
  }
  if (frames.size() > (size_t)kMaxScriptCodecFrames)
  {
    fop->setError("format codec: too many frames (%d)\n", (int)frames.size());
    return false;
  }

  // Validate every frame up front: the sprite has one size and one
  // pixel format, and this codec doesn't go through
  // FileOp::sequenceImage (which validates the dimensions).
  int w = 0, h = 0;
  doc::PixelFormat pixelFormat = doc::IMAGE_RGB;
  bool indexedTransparent = false;
  std::vector<const FrameFormat*> formats;
  for (size_t i = 0; i < frames.size(); ++i)
  {
    auto& frame = frames[i];
    if (!frame.isObject())
    {
      fop->setError("format codec: frame %d is not an object\n", (int)i + 1);
      return false;
    }
    auto& d = frame.object();
    if (!d.contains("width") || !d["width"].isNumber() || !d.contains("height") || !d["height"].isNumber())
    {
      fop->setError("format codec: frame %d: missing width/height\n", (int)i + 1);
      return false;
    }
    int fw = 0, fh = 0;
    if (!getInt(d["width"], 1, kMaxFileImageDimension, fw) || !getInt(d["height"], 1, kMaxFileImageDimension, fh))
    {
      fop->setError("format codec: frame %d: bad frame dimensions\n", (int)i + 1);
      return false;
    }
    if (i == 0)
    {
      w = fw;
      h = fh;
    }
    else if (fw != w || fh != h)
    {
      fop->setError("format codec: frames have different sizes\n");
      return false;
    }

    const std::string formatName = d.contains("format") ? d["format"].string() : "";
    const FrameFormat* ff = findLoadFormat(formatName);
    if (!ff)
    {
      fop->setError("format codec: unknown frame format \"%s\"\n", formatName.c_str());
      return false;
    }
    const doc::PixelFormat pf = docFormat(*ff);
    if (i == 0)
      pixelFormat = pf;
    else if (pf != pixelFormat || (pf == doc::IMAGE_RGB && ff->hasAlpha != formats[0]->hasAlpha))
    {
      fop->setError("format codec: frames have different pixel formats\n");
      return false;
    }
    formats.push_back(ff);

    if (!d.contains("data") || !d["data"].isByteArray())
    {
      fop->setError("format codec: frame %d: missing pixel data\n", (int)i + 1);
      return false;
    }
    if (d["data"].byteArray().size() != (size_t)fw * (size_t)fh * (size_t)ff->bytesPerPixel)
    {
      fop->setError("format codec: frame %d: data size mismatch\n", (int)i + 1);
      return false;
    }

    if (pf == doc::IMAGE_INDEXED)
    {
      size_t paletteSize = 256;
      if (d.contains("palette"))
      {
        if (!d["palette"].isArray() || d["palette"].array().empty() || d["palette"].array().size() > 256)
        {
          fop->setError("format codec: frame %d: bad palette\n", (int)i + 1);
          return false;
        }
        paletteSize = d["palette"].array().size();
        for (auto& entry : d["palette"].array())
        {
          int unused;
          if (!getInt(entry, 0, 4294967295.0, unused))
          {
            fop->setError("format codec: frame %d: bad palette entry\n", (int)i + 1);
            return false;
          }
        }
      }
      if (d.contains("transparentIndex"))
      {
        int ti = 0;
        if (!getInt(d["transparentIndex"], 0, (double)paletteSize - 1, ti))
        {
          fop->setError("format codec: frame %d: bad transparent index\n", (int)i + 1);
          return false;
        }
        indexedTransparent = true;
      }
    }
  }

  // Build the document (the direct-load pattern of the FLI/GIF
  // loaders: one sprite, one layer, one cel per frame).
  std::unique_ptr<Sprite> sprite(new Sprite(pixelFormat, w, h, 256));
  LayerImage* layer = new LayerImage(sprite.get());
  sprite->folder()->addLayer(layer);
  sprite->setTotalFrames((frame_t)frames.size());

  // A frame without alpha becomes an opaque background layer (the
  // sequence-load semantics of FileOp::operateLoadTryFormat).
  bool hasAlpha = (pixelFormat == doc::IMAGE_INDEXED) ? indexedTransparent : formats[0]->hasAlpha;
  if (!hasAlpha)
    layer->configureAsBackground();

  const int bps = (pixelFormat == doc::IMAGE_INDEXED) ? 1 : 4;
  for (size_t i = 0; i < frames.size(); ++i)
  {
    auto& d = frames[i].object();
    const FrameKind kind = formats[i]->kind;
    auto& bytes = d["data"].byteArray();

    ImageRef image(Image::create(pixelFormat, w, h));
    int stride = image->getRowStrideSize();
    uint8_t* dst = image->getPixelAddress(0, 0);
    const uint8_t* src = bytes.data();
    if (pixelFormat == doc::IMAGE_INDEXED || kind == FrameKind::Rgba)
    {
      for (int y = 0; y < h; ++y)
        std::memcpy(dst + (size_t)y * stride, src + (size_t)y * w * bps, (size_t)w * bps);
    }
    else if (kind == FrameKind::Rgb)
    {
      for (int y = 0; y < h; ++y)
      {
        uint32_t* row = (uint32_t*)(dst + (size_t)y * stride);
        const uint8_t* s = src + (size_t)y * w * 3;
        for (int x = 0; x < w; ++x)
          row[x] = rgba(s[x * 3], s[x * 3 + 1], s[x * 3 + 2], 255);
      }
    }
    else
    { // FrameKind::Graya
      for (int y = 0; y < h; ++y)
      {
        uint32_t* row = (uint32_t*)(dst + (size_t)y * stride);
        const uint8_t* s = src + (size_t)y * w * 2;
        for (int x = 0; x < w; ++x)
          row[x] = rgba(s[x * 2], s[x * 2], s[x * 2], s[x * 2 + 1]);
      }
    }

    if (pixelFormat == doc::IMAGE_INDEXED)
    {
      if (d.contains("palette"))
      {
        auto& palette = d["palette"].array();
        auto pal = Palette::create((int)palette.size());
        for (size_t c = 0; c < palette.size(); ++c)
          pal->setEntry((int)c, (doc::color_t)palette[c].number());
        pal->setFrame((frame_t)i);
        sprite->setPalette(*pal, true);
      }
      if (d.contains("transparentIndex"))
        sprite->setTransparentColor((int)d["transparentIndex"].number());
    }

    auto cel = std::make_shared<Cel>((frame_t)i, image);
    layer->addCel(cel);
    fop->setProgress((double)(i + 1) / (double)frames.size());
  }

  fop->createDocument(sprite.release());
  return true;
}

bool ScriptFileFormat::onSave(FileOp* fop)
{
  const Sprite* sprite = fop->document()->sprite();
  const int w = sprite->width();
  const int h = sprite->height();
  const doc::PixelFormat format = sprite->pixelFormat();
  const frame_t total = sprite->totalFrames();

  // Pre-render every frame (the direct save path has no per-frame
  // pre-render; adapt the sequence loop of FileOp::operate) and build
  // the descriptors in the sprite's native pixel format.
  JSON::Value frames;
  render::Render render;
  ImageRef bmp(Image::create(format, w, h));
  for (frame_t frame(0); frame < total; ++frame)
  {
    render.renderSprite(bmp.get(), sprite, frame);

    JSON::Value desc;
    auto& d = desc.object();
    d["width"] = (double)w;
    d["height"] = (double)h;

    std::vector<uint8_t> bytes;
    if (format == doc::IMAGE_BITMAP)
    {
      // 1-bit: expand to RGBA (0/255, alpha 255)
      d["format"] = std::string(kFormatRgba);
      bytes.resize((size_t)w * (size_t)h * 4);
      for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
          uint8_t g = (bmp->getPixel(x, y) != 0) ? 255 : 0;
          auto i = ((size_t)y * (size_t)w + (size_t)x) * 4;
          bytes[i + 0] = g;
          bytes[i + 1] = g;
          bytes[i + 2] = g;
          bytes[i + 3] = 255;
        }
    }
    else
    {
      d["format"] = std::string(saveFormatName(format));
      if (format == doc::IMAGE_GRAYSCALE)
      {
        // 2-byte pixels (value + alpha): GRAY keeps the value byte
        bytes.resize((size_t)w * (size_t)h);
        for (int y = 0; y < h; ++y)
        {
          const uint16_t* row = (const uint16_t*)bmp->getPixelAddress(0, y);
          uint8_t* dst = &bytes[(size_t)y * (size_t)w];
          for (int x = 0; x < w; ++x)
            dst[x] = graya_getv(row[x]);
        }
      }
      else if (format == doc::IMAGE_INDEXED)
      {
        // 1-byte pixels: raw indices (the palette is sent alongside)
        bytes.resize((size_t)w * (size_t)h);
        for (int y = 0; y < h; ++y)
          std::memcpy(&bytes[(size_t)y * (size_t)w], bmp->getPixelAddress(0, y), (size_t)w);
      }
      else
      { // IMAGE_RGB
        bytes.resize((size_t)w * (size_t)h * 4);
        for (int y = 0; y < h; ++y)
          std::memcpy(&bytes[(size_t)y * (size_t)w * 4], bmp->getPixelAddress(0, y), (size_t)w * 4);
      }
    }
    d["data"] = JSON::Value{std::make_shared<std::vector<uint8_t>>(std::move(bytes))};

    if (format == doc::IMAGE_INDEXED)
    {
      const Palette* palette = sprite->palette(frame);
      JSON::Value paletteArr;
      for (int c = 0; c < palette->size(); ++c)
        paletteArr.array().push_back((double)palette->getEntry(c));
      d["palette"] = std::move(paletteArr);
      int ti = (int)sprite->transparentColor();
      if (ti >= 0 && ti < palette->size())
        d["transparentIndex"] = (double)ti;
    }

    frames.array().push_back(std::move(desc));
  }

  JSON::Value result = dispatchCodec(fop, "format_save", std::move(frames));
  if (isCodecError(result))
  {
    fop->setError("Error: %s\n", result["error"].string().c_str());
    return false;
  }
  if (!result.isByteArray())
  {
    fop->setError("format codec: the save reply must be a byte array\n");
    return false;
  }

  auto& bytes = result.byteArray();
  base::FileHandle handle(base::open_file_with_exception(fop->filename(), "wb"));
  if (!bytes.empty() && std::fwrite(bytes.data(), 1, bytes.size(), handle.get()) != bytes.size())
  {
    fop->setError("Error writing file \"%s\"\n", fop->filename().c_str());
    return false;
  }
  return true;
}

} // namespace app
