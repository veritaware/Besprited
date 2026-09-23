// LibreSprite | Copyright (C) 2023-2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

// Shared helpers for the delta `Extension`-based script API
//
// Doc objects (Document/Layer/Image/Palette/Cel) are owned by the document
// model, not by us. `document.close()` actually frees a document's whole
// object graph (see DocumentExtension), so a JS wrapper can never be allowed
// to hold a raw non-owning pointer into it -- that was the old approach, and
// it was undefined behaviour on next use once the document was closed.
//
// Instead, `ScriptRef<T>` resolves by `doc::ObjectId` on every call, via the
// global id map every `doc::Object` already registers itself in
// (`doc::get_object` / `doc::get<T>`). If the underlying object no longer
// exists, `get()` throws, which the delta/QuickJS bridge turns into a
// catchable JS error at the call site instead of dereferencing freed memory.

#pragma once

#include "doc/object.h"

#include <memory>
#include <stdexcept>
#include <unordered_map>

namespace script_api
{

// Stateless marker "sites" for proxy classes whose methods re-resolve the
// active document on every access (instead of capturing a snapshot).
struct SpriteSite
{
};
struct SelectionSite
{
};
struct PixelColorSite
{
};

// A handle into a doc::Object-derived instance, resolved by id rather than
// held by pointer, so it never dangles after the owning document is closed.
template <typename T> class ScriptRef
{
public:
  explicit ScriptRef(doc::ObjectId id)
    : m_id(id)
  {
  }

  // Throws std::runtime_error (surfaced to the script as a catchable error)
  // if the object was freed, e.g. by a document.close() in the meantime.
  T& get() const
  {
    if (T* obj = doc::get<T>(m_id))
      return *obj;
    throw std::runtime_error{
        "Object no longer exists (its document was probably closed)"};
  }

  doc::ObjectId id() const { return m_id; }

private:
  doc::ObjectId m_id;
};

// Wrap a non-owning raw pointer in a shared_ptr with a no-op deleter, for
// wrappers whose lifetime is unrelated to the document model (e.g. the
// persistent `CommandObject` behind the `command` global, which is always
// JS-owned already). Doc objects (Document/Layer/Image/Palette/Cel) must use
// `wrap<T>` below instead, since they can be freed out from under a script.
template <typename T> std::shared_ptr<T> wrapNonOwning(T* p)
{
  return std::shared_ptr<T>(p, [](T*) {});
}

// Wrap a doc::Object-derived pointer in a ScriptRef<T>, cached by ObjectId so
// repeated wraps of the same object return the same ScriptRef instance. This
// matters for identity: the interpreter dedups JS wrappers by the address of
// the wrapped native object (see QuickJSInterpreter's `wrappers` map keyed on
// the shared_ptr's pointee), so `sprite.layer(0) === sprite.layer(0)` only
// holds if both calls hand back the same ScriptRef address. Caching by id
// (never by address) also means a freed-and-reallocated object can't inherit
// a stale wrapper.
template <typename T> std::shared_ptr<ScriptRef<T>> wrap(T* p)
{
  if (!p)
    return nullptr;

  static std::unordered_map<doc::ObjectId, std::weak_ptr<ScriptRef<T>>> cache;

  const doc::ObjectId id = p->id();
  auto it = cache.find(id);
  if (it != cache.end())
  {
    if (auto existing = it->second.lock())
      return existing;
    cache.erase(it);
  }

  auto ref = std::make_shared<ScriptRef<T>>(id);
  cache.emplace(id, ref);
  return ref;
}

} // namespace script_api
