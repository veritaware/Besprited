// CSS Library
// Aseprite  | Copyright (C) 2013 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "css/compound_style.h"

#include "css/sheet.h"

#include <utility>

namespace css
{

CompoundStyle::CompoundStyle(Sheet* sheet, std::string name)
  : m_sheet(sheet)
  , m_name(std::move(name))
{
  update();
}

void CompoundStyle::update()
{
  deleteQueries();

  const Style* style = m_sheet->getStyle(m_name);
  if (style)
    m_normal = m_sheet->query(*style);
}

CompoundStyle::~CompoundStyle()
{
  deleteQueries();
}

void CompoundStyle::deleteQueries()
{
  for (const auto& entry : m_queries)
    delete entry.second;
  m_queries.clear();
}

const Value& CompoundStyle::operator[](const Rule& rule) const
{
  return m_normal[rule];
}

const Query& CompoundStyle::operator[](const States& states) const
{
  const auto it = m_queries.find(states);

  if (it != m_queries.end())
    return *it->second;
  else
  {
    const Style* style = m_sheet->getStyle(m_name);
    if (style == nullptr)
      return m_normal;

    auto* newQuery = new Query(m_sheet->query(StatefulStyle(*style, states)));
    m_queries[states] = newQuery;
    return *newQuery;
  }
}

} // namespace css
