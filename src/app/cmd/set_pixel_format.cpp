// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/set_pixel_format.h"

#include "app/cmd/remove_palette.h"
#include "app/cmd/replace_image.h"
#include "app/cmd/set_cel_opacity.h"
#include "app/cmd/set_palette.h"
#include "app/document.h"
#include "doc/cel.h"
#include "doc/cels_range.h"
#include "doc/document.h"
#include "doc/document_event.h"
#include "doc/layer.h"
#include "doc/palette.h"
#include "doc/sprite.h"
#include "render/quantization.h"

#include <memory>

namespace app::cmd
{

using namespace doc;

SetPixelFormat::SetPixelFormat(Sprite* sprite, const PixelFormat newFormat,
                               const DitheringMethod dithering)
  : WithSprite(sprite)
  , m_oldFormat(sprite->pixelFormat())
  , m_newFormat(newFormat)
  , m_dithering(dithering)
{
  if (sprite->pixelFormat() == newFormat)
    return;

  for (const auto cel : sprite->uniqueCels())
  {
    const ImageRef old_image = cel->imageRef();
    const ImageRef new_image(render::convert_pixel_format(
        old_image.get(), nullptr, newFormat, m_dithering,
        sprite->rgbMap(cel->frame()), sprite->palette(cel->frame()),
        cel->layer()->isBackground(), old_image->maskColor()));

    m_seq.add(new ReplaceImage(sprite, old_image, new_image));
  }

  // Set all cels opacity to 100% if we are converting to indexed.
  // TODO remove this
  if (newFormat == IMAGE_INDEXED)
  {
    for (const auto cel : sprite->uniqueCels())
    {
      if (cel->opacity() < 255)
        m_seq.add(new SetCelOpacity(cel, 255));
    }
  }

  // When we are converting to grayscale color mode, we've to destroy
  // all palettes and put only one grayscaled-palette at the first
  // frame.
  if (newFormat == IMAGE_GRAYSCALE)
  {
    // Add cmds to revert all palette changes.
    for (const PalettesList palettes = sprite->getPalettes();
         auto& pal : palettes)
      if (pal->frame() != 0)
        m_seq.add(new RemovePalette(sprite, *pal));

    const auto graypal = Palette::createGrayscale();
    m_seq.add(new SetPalette(sprite, 0, *graypal));
  }
}

void SetPixelFormat::onExecute()
{
  m_seq.execute(context());
  setFormat(m_newFormat);
}

void SetPixelFormat::onUndo()
{
  m_seq.undo();
  setFormat(m_oldFormat);
}

void SetPixelFormat::onRedo()
{
  m_seq.redo();
  setFormat(m_newFormat);
}

void SetPixelFormat::setFormat(const PixelFormat format)
{
  Sprite* sprite = this->sprite();

  sprite->setPixelFormat(format);
  sprite->incrementVersion();

  // Regenerate extras
  dynamic_cast<Document*>(sprite->document())
      ->setExtraCel(ExtraCelRef(nullptr));

  // Generate notification
  DocumentEvent ev(sprite->document());
  ev.sprite(sprite);
  sprite->document()->notifyObservers<DocumentEvent&>(
      &DocumentObserver::onPixelFormatChanged, ev);
}

} // namespace app::cmd
