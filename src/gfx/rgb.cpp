// Gfx Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#include "gfx/rgb.h"
#include "gfx/hsv.h"
#include <cmath>

namespace gfx
{

using namespace std;

// Reference: http://en.wikipedia.org/wiki/HSL_and_HSV
Rgb::Rgb(const Hsv& hsv)
{
  const double chroma = hsv.value() * hsv.saturation();
  const double hue_prime = hsv.hue() / 60.0;
  const double x = chroma * (1.0 - fabs(fmod(hue_prime, 2.0) - 1.0));
  double r, g, b;

  r = g = b = 0.0;

  switch (static_cast<int>(hue_prime))
  {

  case 6:
  case 0:
    r = chroma;
    g = x;
    break;
  case 1:
    r = x;
    g = chroma;
    break;

  case 2:
    g = chroma;
    b = x;
    break;
  case 3:
    g = x;
    b = chroma;
    break;

  case 4:
    b = chroma;
    r = x;
    break;
  case 5:
    b = x;
    r = chroma;
    break;
  default:
    break;
  }

  const double m = hsv.value() - chroma;
  r += m;
  g += m;
  b += m;

  m_red = static_cast<int>(std::lround(r * 255.0));
  m_green = static_cast<int>(std::lround(g * 255.0));
  m_blue = static_cast<int>(std::lround(b * 255.0));
}

int Rgb::maxComponent() const
{
  if (m_red > m_green)
    return (m_red > m_blue) ? m_red : m_blue;
  else
    return (m_green > m_blue) ? m_green : m_blue;
}

int Rgb::minComponent() const
{
  if (m_red < m_green)
    return (m_red < m_blue) ? m_red : m_blue;
  else
    return (m_green < m_blue) ? m_green : m_blue;
}

} // namespace gfx
