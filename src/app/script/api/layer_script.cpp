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
#include "doc/layer.h"

#include <memory>

using LayerRef = script_api::ScriptRef<doc::Layer>;

// `Layer` wraps a `doc::Layer` (a snapshot of a specific layer, obtained via
// `sprite.layer(i)`) by id -- see ScriptRef.
class LayerExtension : public Extension
{
public:
  LayerExtension()
  {
    auto& clazz = addClass<void, LayerRef>("Layer");
    clazz.setConstructor() = []() -> std::shared_ptr<void>
    { throw std::runtime_error{"Layer cannot be constructed directly"}; };

    clazz.addGetter("name") = [](LayerRef& ref) -> JSON::Value
    { return std::string{ref.get().name()}; };
    clazz.addSetter("name") = [](LayerRef& ref, JSON::Value& v)
    { ref.get().setName(static_cast<std::string>(v)); };

    clazz.addGetter("isImage") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isImage(); };
    clazz.addGetter("isBackground") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isBackground(); };
    clazz.addGetter("isTransparent") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isTransparent(); };

    clazz.addGetter("isVisible") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isVisible(); };
    clazz.addSetter("isVisible") = [](LayerRef& ref, JSON::Value& v)
    { ref.get().setVisible(static_cast<bool>(v)); };

    clazz.addGetter("isEditable") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isEditable(); };
    clazz.addSetter("isEditable") = [](LayerRef& ref, JSON::Value& v)
    { ref.get().setEditable(static_cast<bool>(v)); };

    clazz.addGetter("isMovable") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isMovable(); };
    clazz.addGetter("isContinuous") = [](LayerRef& ref) -> JSON::Value
    { return ref.get().isContinuous(); };
    clazz.addGetter("flags") = [](LayerRef& ref) -> JSON::Value
    { return (double)static_cast<int>(ref.get().flags()); };

    clazz.addGetter("celCount") = [](LayerRef& ref) -> JSON::Value
    {
      auto& layer = ref.get();
      if (layer.isImage())
        return (double)static_cast<doc::LayerImage*>(&layer)->getCelsCount();
      return 0.0;
    };

    clazz.addMethod("cel") = [](LayerRef& ref, double i) -> JSON::Value
    { return JSON::makeNative(script_api::wrap(ref.get().cel((doc::frame_t)i).get())); };
  }
};

static di::provide<Extension, LayerExtension> x{"layer"};
