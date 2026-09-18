// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include "css/query.h"
#include "css/rule.h"
#include "css/state.h"
#include "css/stateful_style.h"

namespace css
{

class Sheet;

class CompoundStyle
{
public:
  CompoundStyle(Sheet* sheet, std::string name);
  ~CompoundStyle();

  void update();

  const Value& operator[](const Rule& rule) const;
  const Query& operator[](const States& states) const;

private:
  using QueriesMap = std::map<States, Query*>;

  void deleteQueries();

  Sheet* m_sheet;
  std::string m_name;
  Query m_normal;
  mutable QueriesMap m_queries;
};

} // namespace css
