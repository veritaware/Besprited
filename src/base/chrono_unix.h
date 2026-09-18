// Base Library
// Aseprite  | Copyright (C) 2001-2013, 2015 David Capello
// Besprited | Copyright (C) 2026            Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include <ctime>
#include <sys/time.h>

class base::Chrono::ChronoImpl
{
public:
  ChronoImpl() { reset(); }

  void reset() { gettimeofday(&m_point, nullptr); }

  double elapsed() const
  {
    struct timeval now;
    gettimeofday(&now, nullptr);
    return (double)(now.tv_sec + (double)now.tv_usec / 1000000) -
           (double)(m_point.tv_sec + (double)m_point.tv_usec / 1000000);
  }

private:
  struct timeval m_point;
};
