// LibreSprite | Copyright (C) 2021-2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "delta/Extension.hpp"
#include "delta/JSON.hpp"
#include "di.hpp"
#include "app/script/api/script_api_common.h"

#include "base/base64.h"
#include "doc/image.h"
#include "she/surface.h"
#include "she/system.h"
#include "ui/manager.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <vector>

using ImageRef = script_api::ScriptRef<doc::Image>;

class ImageExtension : public Extension
{
public:
  ImageExtension()
  {
    auto& clazz = addClass<void, ImageRef>("Image");
    clazz.setConstructor() = []() -> std::shared_ptr<void>
    { throw std::runtime_error{"Image cannot be constructed directly"}; };

    clazz.addGetter("width") = [](ImageRef& ref) -> JSON::Value
    { return (double)ref.get().width(); };
    clazz.addGetter("height") = [](ImageRef& ref) -> JSON::Value
    { return (double)ref.get().height(); };
    clazz.addGetter("stride") = [](ImageRef& ref) -> JSON::Value
    { return (double)ref.get().getRowStrideSize(); };
    clazz.addGetter("format") = [](ImageRef& ref) -> JSON::Value
    { return (double)ref.get().pixelFormat(); };

    clazz.addMethod("getPixel") = [](ImageRef& ref, double x,
                                     double y) -> JSON::Value
    { return (double)ref.get().getPixel((int)x, (int)y); };

    clazz.addMethod("putPixel") = [](ImageRef& ref, double x, double y,
                                     double color) -> JSON::Value
    {
      auto& img = ref.get();
      if ((unsigned)x < (unsigned)img.width() &&
          (unsigned)y < (unsigned)img.height())
        img.putPixel((int)x, (int)y, (doc::color_t)color);
      return {};
    };

    clazz.addMethod("clear") = [](ImageRef& ref, double color) -> JSON::Value
    {
      ref.get().clear((doc::color_t)color);
      return {};
    };

    clazz.addMethod("putImageData") = [](ImageRef& ref,
                                         JSON::Value& data) -> JSON::Value
    {
      auto& img = ref.get();
      auto& bytes = data.byteArray();
      if (bytes.size() !=
          static_cast<std::size_t>(img.getRowStrideSize()) * img.height())
      {
        std::cout << "Data size mismatch: " << bytes.size() << '\n';
        return {};
      }
      std::memcpy(img.getPixelAddress(0, 0), bytes.data(), bytes.size());
      if (auto* mgr = ui::Manager::getDefault())
        mgr->invalidate();
      return {};
    };

    clazz.addMethod("getImageData") = [](ImageRef& ref) -> JSON::Value
    {
      auto& img = ref.get();
      auto* addr = img.getPixelAddress(0, 0);
      const std::size_t size =
          static_cast<std::size_t>(img.getRowStrideSize()) * img.height();
      auto vec = std::make_shared<std::vector<uint8_t>>(addr, addr + size);
      return JSON::Value{vec};
    };

    clazz.addMethod("getPNGData") = [](ImageRef& ref) -> JSON::Value
    {
      auto& img = ref.get();
      auto w = img.width();
      auto h = img.height();
      const std::shared_ptr<she::Surface> surface{
          she::instance()->createRgbaSurface(w, h),
          [](she::Surface* s) { s->dispose(); }};
      if (!surface)
        return std::string{};

      for (auto y = 0; y < h; ++y)
        for (auto x = 0; x < w; ++x)
          surface->putPixel(img.getPixel(x, y), x, y);

      std::string encoded;
      base::encode_base64(she::instance()->encodeSurfaceAsPNG(surface.get()),
                          encoded);
      return std::string{"data:image/png;base64,"} + encoded;
    };
  }
};

static di::provide<Extension, ImageExtension> x{"image"};
