// Document Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

namespace doc
{

class Document;
class Site;

class ContextObserver
{
public:
  virtual ~ContextObserver() {}
  virtual void onActiveSiteChange(const Site& site) {}
};

} // namespace doc
