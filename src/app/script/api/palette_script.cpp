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
#include "app/script/app_scripting.h"

#include "app/modules/palettes.h"
#include "doc/color.h"
#include "doc/palette.h"
#include "ui/manager.h"

#include <memory>

using PaletteRef = script_api::ScriptRef<doc::Palette>;

class PaletteExtension : public Extension
{
public:
  PaletteExtension()
  {
    auto& clazz = addClass<void, PaletteRef>("Palette");
    clazz.setConstructor() = []() -> std::shared_ptr<void>
    { throw std::runtime_error{"Palette cannot be constructed directly"}; };

    clazz.addGetter("length") = [](PaletteRef& ref) -> JSON::Value
    { return (double)ref.get().size(); };
    clazz.addSetter("length") = [](PaletteRef& ref, JSON::Value& v)
    {
      auto& pal = ref.get();
      pal.resize(static_cast<int>(v));
      schedulePaletteUpdate(&pal);
    };

    clazz.addMethod("get") = [](PaletteRef& ref, double i) -> JSON::Value
    { return (double)ref.get().getEntry((int)i); };

    // `set` is variadic: `set(i, color)`, `set(i, r, g, b)`, or `set(i, r, g,
    // b, a)`. Missing trailing args are padded with `undefined`; the form is
    // disambiguated by which trailing args are defined.
    clazz.addMethod("set") = [](PaletteRef& ref, int i, JSON::Value& a,
                                JSON::Value& b, JSON::Value& c,
                                JSON::Value& d) -> JSON::Value
    {
      auto& pal = ref.get();
      if (i < 0 || i >= pal.size())
        return {};
      doc::color_t color;
      if (b.isUndefined())
      {
        // set(i, color)
        color = (doc::color_t) static_cast<int>(a);
      }
      else if (d.isUndefined())
      {
        // set(i, r, g, b)
        color = doc::rgba((uint8_t)static_cast<int>(a),
                          (uint8_t)static_cast<int>(b),
                          (uint8_t)static_cast<int>(c), 0xFF);
      }
      else
      {
        // set(i, r, g, b, a)
        color = doc::rgba(
            (uint8_t)static_cast<int>(a), (uint8_t)static_cast<int>(b),
            (uint8_t)static_cast<int>(c), (uint8_t)static_cast<int>(d));
      }
      pal.setEntry(i, color);
      schedulePaletteUpdate(&pal);
      return {};
    };
  }

private:
  // Defer the version increment until after the script's eval so that several
  // palette changes in one run coalesce into a single UI refresh (matching the
  // old engine's afterEval behavior).
  static void schedulePaletteUpdate(doc::Palette* pal)
  {
    app::AppScripting::afterEval(
        [pal]
        {
          pal->incrementVersion();
          app::set_current_palette(pal, true);
          if (auto* mgr = ui::Manager::getDefault())
            mgr->invalidate();
        });
  }
};

static di::provide<Extension, PaletteExtension> x{"palette"};
