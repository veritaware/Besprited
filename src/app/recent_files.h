// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#pragma once

#include "base/recent_items.h"
#include "base/signal.h"

#include <string>

namespace app
{

class RecentFiles
{
public:
  using List = base::RecentItems<std::string>;
  using iterator = List::iterator;
  using const_iterator = List::const_iterator;

  // Iterate through recent files.
  const_iterator files_begin() { return m_files.begin(); }
  const_iterator files_end() { return m_files.end(); }

  // Iterate through recent paths.
  const_iterator paths_begin() { return m_paths.begin(); }
  const_iterator paths_end() { return m_paths.end(); }

  RecentFiles();
  ~RecentFiles();

  void addRecentFile(const char* filename);
  void removeRecentFile(const char* filename);

  base::Signal0<void> Changed;

private:
  std::string normalizePath(std::string fn);

  List m_files;
  List m_paths;
};

} // namespace app
