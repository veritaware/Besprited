// Document Library
// Aseprite  | Copyright (C) 2001-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "doc/context.h"

#include "base/debug.h"
#include "doc/site.h"

namespace doc
{

Context::Context()
  : m_docs(this)
  , m_activeDoc(nullptr)
{
  m_docs.addObserver(this);
}

Context::~Context()
{
  m_docs.removeObserver(this);
}

Site Context::activeSite() const
{
  Site site;
  onGetActiveSite(&site);
  return site;
}

Document* Context::activeDocument() const
{
  Site site;
  onGetActiveSite(&site);
  return site.document();
}

void Context::notifyActiveSiteChanged()
{
  const Site site = activeSite();
  notifyObservers<const Site&>(&ContextObserver::onActiveSiteChange, site);
}

void Context::onGetActiveSite(Site* site) const
{
  ASSERT(false);
}

void Context::onAddDocument(Document* doc)
{
  // Do nothing
}

void Context::onRemoveDocument(Document* doc)
{
  // Do nothing
}

} // namespace doc
