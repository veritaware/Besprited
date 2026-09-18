// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/cmd/replace_image.h"

#include "doc/cel.h"
#include "doc/cels_range.h"
#include "doc/image.h"
#include "doc/image_ref.h"
#include "doc/sprite.h"

namespace app::cmd
{

using namespace doc;

ReplaceImage::ReplaceImage(const Sprite* sprite, const ImageRef& oldImage,
                           const ImageRef& newImage)
  : WithSprite(sprite)
  , m_oldImageId(oldImage->id())
  , m_newImageId(newImage->id())
  , m_newImage(newImage)
{
}

void ReplaceImage::onExecute()
{
  // Save old image in m_copy. We cannot keep an ImageRef to this
  // image, because there are other undo branches that could try to
  // modify/re-add this same image ID
  const ImageRef oldImage = sprite()->getImageRef(m_oldImageId);
  ASSERT(oldImage);
  m_copy.reset(Image::createCopy(oldImage.get()));

  replaceImage(m_oldImageId, m_newImage);
  m_newImage.reset();
}

void ReplaceImage::onUndo()
{
  const ImageRef newImage = sprite()->getImageRef(m_newImageId);
  ASSERT(newImage);
  ASSERT(!sprite()->getImageRef(m_oldImageId));
  m_copy->setId(m_oldImageId);

  replaceImage(m_newImageId, m_copy);
  m_copy.reset(Image::createCopy(newImage.get()));
}

void ReplaceImage::onRedo()
{
  const ImageRef oldImage = sprite()->getImageRef(m_oldImageId);
  ASSERT(oldImage);
  ASSERT(!sprite()->getImageRef(m_newImageId));
  m_copy->setId(m_newImageId);

  replaceImage(m_oldImageId, m_copy);
  m_copy.reset(Image::createCopy(oldImage.get()));
}

void ReplaceImage::replaceImage(const ObjectId oldId,
                                const ImageRef& newImage) const
{
  Sprite* spr = sprite();

  for (const auto cel : spr->uniqueCels())
  {
    if (cel->image()->id() == oldId)
      cel->data()->incrementVersion();
  }

  spr->replaceImage(oldId, newImage);
}

} // namespace app::cmd
