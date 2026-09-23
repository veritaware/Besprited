// LibreSprite | Copyright (C) 2023 LibreSprite contributors
// Besprited   | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//

#include "doc/color.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/document.h"
#include "app/file/file.h"
#include "app/file/file_format.h"
#include "app/file/format_options.h"
#include "base/cfile.h"
#include "base/file_handle.h"
#include "doc/doc.h"
#include "doc/pixel_format.h"
#include "render/render.h"
#include "she/system.h"
#include "she/surface.h"
#include "she/surface_format.h"

#include <chrono>
#include <future>
#include <memory>
#include <thread>

namespace app
{

using namespace base;

class SheFormat : public FileFormat
{
  const char* onGetName() const override { return "she"; }
  const char* onGetExtensions() const override { return ""; }
  int onGetFlags() const override
  {
    return FILE_SUPPORT_LOAD | FILE_SUPPORT_SEQUENCES;
  }

  bool onSave(FileOp* fop) override { return false; }

  bool onLoad(FileOp* fop) override;
};

static FileFormat::Regular<SheFormat> ff{"she"};

bool SheFormat::onLoad(FileOp* fop)
{
  try
  {
    // loadRgbaSurface() delegates to SDL_image for any format we don't
    // have a native decoder for. Fuzzing found a malformed GIF that made
    // SDL_image's own decoder attempt a ~3.8 GB allocation, plus separate
    // multi-minute hangs on other malformed inputs (issue #235) - bugs in
    // SDL_image itself, outside this repo's control, with no hook to
    // validate the file before it decodes. Run the decode on its own
    // thread with a wall-clock timeout so a malformed file can wedge
    // that thread but not this one; there's no safe way to cancel a
    // decode mid-flight, so on timeout we detach it and give up waiting
    // rather than hang the app.
    constexpr auto kLoadTimeout = std::chrono::seconds(15);

    auto promise =
        std::make_shared<std::promise<std::shared_ptr<she::Surface>>>();
    std::future<std::shared_ptr<she::Surface>> future = promise->get_future();
    std::string filename = fop->filename();

    std::thread worker(
        [promise, filename]
        {
          try
          {
            promise->set_value(std::shared_ptr<she::Surface>(
                she::instance()->loadRgbaSurface(filename.c_str())));
          }
          catch (...)
          {
            promise->set_exception(std::current_exception());
          }
        });

    if (future.wait_for(kLoadTimeout) != std::future_status::ready)
    {
      worker.detach();
      return false;
    }
    worker.join();

    auto surface = future.get();
    if (!surface)
      return false;
    auto width = surface->width();
    auto height = surface->height();
    // sequenceImage() rejects unreasonable dimensions (see
    // kMaxFileImageDimension, issue #219) - bail before the per-pixel copy
    // loop below rather than dereferencing a null image.
    Image* image = fop->sequenceImage(IMAGE_RGB, width, height);
    if (!image)
      return false;
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        auto c = surface->getPixel(x, y);
        auto r = gfx::getr(c);
        auto g = gfx::getg(c);
        auto b = gfx::getb(c);
        auto a = gfx::geta(c);
        put_pixel(image, x, y, doc::rgba(r, g, b, a));
      }
    }
    return true;
  }
  catch (...)
  {
  }

  return false;
}

} // namespace app
