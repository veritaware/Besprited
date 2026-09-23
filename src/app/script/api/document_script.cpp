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
#include "app/document_access.h"
#include "doc/document.h"

#include <memory>
#include <stdexcept>

using DocumentRef = script_api::ScriptRef<doc::Document>;

// `Document` wraps a `doc::Document` by id (see ScriptRef). It is not
// constructible from JS (the constructor throws); instances are produced by
// the app-level API (`app.activeDocument`, `app.open`).
//
// `document.sprite` returns the `Sprite` proxy (active-document semantics,
// matching the `sprite` global) rather than a per-document snapshot
class DocumentExtension : public Extension
{
public:
  DocumentExtension()
  {
    auto& clazz = addClass<void, DocumentRef>("Document");
    clazz.setConstructor() = []() -> std::shared_ptr<void>
    { throw std::runtime_error{"Document cannot be constructed directly"}; };

    clazz.addGetter("sprite") = [](DocumentRef&) -> JSON::Value
    { return JSON::makeNative(std::make_shared<script_api::SpriteSite>()); };

    // Actually frees the Document/Sprite/Layer/Image graph (via
    // DocumentDestroyer), not just unlinking it from the active documents
    // list. Any other JS handle still resolving into this document (a Layer
    // or Image grabbed earlier in the same script) will throw on next use
    // instead of dereferencing freed memory -- see ScriptRef::get().
    clazz.addMethod("close") = [](DocumentRef& ref) -> JSON::Value
    {
      auto* doc = static_cast<app::Document*>(&ref.get());
      auto* ctx = static_cast<app::Context*>(doc->context());
      app::DocumentDestroyer destroyer(ctx, doc, 500);
      destroyer.destroyDocument();
      return true;
    };
  }
};

static di::provide<Extension, DocumentExtension> x{"document"};
