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

#include "doc/cel.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/site.h"

#include <memory>

using CelRef = script_api::ScriptRef<doc::Cel>;

class CelExtension : public Extension
{
public:
  CelExtension()
  {
    auto& clazz = addClass<void, CelRef>("Cel");
    clazz.setConstructor() = []() -> std::shared_ptr<void>
    { throw std::runtime_error{"Cel cannot be constructed directly"}; };

    clazz.addGetter("x") = [](CelRef& ref) -> JSON::Value { return (double)ref.get().x(); };
    clazz.addSetter("x") = [](CelRef& ref, JSON::Value& v)
    {
      auto& cel = ref.get();
      cel.setPosition(static_cast<int>(v), cel.y());
    };

    clazz.addGetter("y") = [](CelRef& ref) -> JSON::Value { return (double)ref.get().y(); };
    clazz.addSetter("y") = [](CelRef& ref, JSON::Value& v)
    {
      auto& cel = ref.get();
      cel.setPosition(cel.x(), static_cast<int>(v));
    };

    // The wrapper re-resolves its site's image on every call (see ImageSite);
    // the site is this cel's (document, layer, frame) slot.
    clazz.addGetter("image") = [](CelRef& ref) -> JSON::Value
    {
      auto& cel = ref.get();
      if (!cel.image())
        return JSON::Value{JSON::Special::Null};
      doc::Site site;
      site.document(cel.document());
      site.sprite(cel.sprite());
      site.layer(cel.layer());
      site.frame(cel.frame());
      return JSON::makeNative(script_api::ImageSite::wrap(site));
    };

    clazz.addGetter("frame") = [](CelRef& ref) -> JSON::Value { return (double)ref.get().frame(); };

    clazz.addMethod("setPosition") = [](CelRef& ref, double x, double y) -> JSON::Value
    {
      ref.get().setPosition((int)x, (int)y);
      return {};
    };
  }
};

static di::provide<Extension, CelExtension> x{"cel"};
