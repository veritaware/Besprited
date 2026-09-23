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

    clazz.addGetter("x") = [](CelRef& ref) -> JSON::Value
    { return (double)ref.get().x(); };
    clazz.addSetter("x") = [](CelRef& ref, JSON::Value& v)
    {
      auto& cel = ref.get();
      cel.setPosition(static_cast<int>(v), cel.y());
    };

    clazz.addGetter("y") = [](CelRef& ref) -> JSON::Value
    { return (double)ref.get().y(); };
    clazz.addSetter("y") = [](CelRef& ref, JSON::Value& v)
    {
      auto& cel = ref.get();
      cel.setPosition(cel.x(), static_cast<int>(v));
    };

    clazz.addGetter("image") = [](CelRef& ref) -> JSON::Value
    { return JSON::makeNative(script_api::wrap(ref.get().image())); };

    clazz.addGetter("frame") = [](CelRef& ref) -> JSON::Value
    { return (double)ref.get().frame(); };

    clazz.addMethod("setPosition") = [](CelRef& ref, double x,
                                        double y) -> JSON::Value
    {
      ref.get().setPosition((int)x, (int)y);
      return {};
    };
  }
};

static di::provide<Extension, CelExtension> x{"cel"};
