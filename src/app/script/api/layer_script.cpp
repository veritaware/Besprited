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

#include "app/document.h"
#include "app/document_api.h"
#include "app/transaction.h"
#include "app/ui_context.h"
#include "doc/cel.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/sprite.h"

#include <memory>
#include <stdexcept>

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

    clazz.addGetter("name") = [](LayerRef& ref) -> JSON::Value { return std::string{ref.get().name()}; };
    clazz.addSetter("name") = [](LayerRef& ref, JSON::Value& v) { ref.get().setName(static_cast<std::string>(v)); };

    clazz.addGetter("isImage") = [](LayerRef& ref) -> JSON::Value { return ref.get().isImage(); };
    clazz.addGetter("isBackground") = [](LayerRef& ref) -> JSON::Value { return ref.get().isBackground(); };
    clazz.addGetter("isTransparent") = [](LayerRef& ref) -> JSON::Value { return ref.get().isTransparent(); };

    clazz.addGetter("isVisible") = [](LayerRef& ref) -> JSON::Value { return ref.get().isVisible(); };
    clazz.addSetter("isVisible") = [](LayerRef& ref, JSON::Value& v) { ref.get().setVisible(static_cast<bool>(v)); };

    clazz.addGetter("isEditable") = [](LayerRef& ref) -> JSON::Value { return ref.get().isEditable(); };
    clazz.addSetter("isEditable") = [](LayerRef& ref, JSON::Value& v) { ref.get().setEditable(static_cast<bool>(v)); };

    clazz.addGetter("isMovable") = [](LayerRef& ref) -> JSON::Value { return ref.get().isMovable(); };
    clazz.addGetter("isContinuous") = [](LayerRef& ref) -> JSON::Value { return ref.get().isContinuous(); };
    clazz.addGetter("flags") = [](LayerRef& ref) -> JSON::Value { return (double)static_cast<int>(ref.get().flags()); };

    clazz.addGetter("celCount") = [](LayerRef& ref) -> JSON::Value
    {
      auto& layer = ref.get();
      if (layer.isImage())
        return (double)static_cast<doc::LayerImage*>(&layer)->getCelsCount();
      return 0.0;
    };

    clazz.addMethod("cel") = [](LayerRef& ref, double i) -> JSON::Value
    { return JSON::makeNative(script_api::wrap(ref.get().cel((doc::frame_t)i).get())); };

    // The ASSERT in DocumentApi::addCel is debug-only, so the wrapper checks
    // the preconditions itself and throws (surfaces as a JS exception).
    clazz.addMethod("createCel") = [](LayerRef& ref, double i) -> JSON::Value
    {
      auto& layer = ref.get();
      if (!layer.isImage())
        throw std::runtime_error{"createCel: not an image layer"};
      auto frame = static_cast<doc::frame_t>(i);
      auto* li = static_cast<doc::LayerImage*>(&layer);
      auto* sprite = li->sprite();
      if (frame < 0 || frame >= sprite->totalFrames())
        throw std::runtime_error{"createCel: frame out of range"};
      if (li->cel(frame) != nullptr)
        throw std::runtime_error{"createCel: a cel already exists at this frame"};

      app::Transaction tx(app::UIContext::instance(), "Script Execution", app::ModifyDocument);
      auto* doc = static_cast<app::Document*>(sprite->document());
      doc::ImageRef image(doc::Image::create(sprite->pixelFormat(), sprite->width(), sprite->height()));
      auto cel = doc->getApi(tx).addCel(li, frame, image);
      tx.commit();
      return JSON::makeNative(script_api::wrap(cel.get()));
    };
  }
};

static di::provide<Extension, LayerExtension> x{"layer"};
