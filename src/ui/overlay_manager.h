// UI Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "ui/base.h"
#include <vector>

namespace she
{
class Surface;
}

namespace ui
{

class Overlay;

class OverlayManager
{
  friend class UISystem; // So it can call destroyInstance() from ~UISystem
  static OverlayManager* m_singleton;

  OverlayManager();
  ~OverlayManager();

public:
  static OverlayManager* instance();

  void addOverlay(Overlay* overlay);
  void removeOverlay(Overlay* overlay);

  void captureOverlappedAreas();
  void restoreOverlappedAreas();
  void drawOverlays();

private:
  static void destroyInstance();

  using OverlayList = std::vector<Overlay*>;
  using iterator = OverlayList::iterator;

  iterator begin() { return m_overlays.begin(); }
  iterator end() { return m_overlays.end(); }

  OverlayList m_overlays;
};

} // namespace ui
