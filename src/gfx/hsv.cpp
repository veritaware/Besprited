// Gfx Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gfx/hsv.h"
#include "gfx/rgb.h"
#include <cmath>

namespace gfx
{

using namespace std;

Hsv::Hsv(double hue, double saturation, double value)
  : m_hue(hue)
  , m_saturation(saturation)
  , m_value(value)
{
  while (m_hue < 0.0)
    m_hue += 360.0;
  m_hue = fmod(hue, 360.0);

  assert(hue >= 0.0 && hue <= 360.0);
  assert(saturation >= 0.0 && saturation <= 1.0);
  assert(value >= 0.0 && value <= 1.0);
}

// Reference: http://en.wikipedia.org/wiki/HSL_and_HSV
Hsv::Hsv(const Rgb& rgb)
{
  const int M = rgb.maxComponent();
  const int m = rgb.minComponent();
  const int c = M - m;
  const double chroma = static_cast<double>(c) / 255.0;
  double hue_prime = 0.0;
  double h, s, v;
  double r, g, b;

  v = static_cast<double>(M) / 255.0;

  if (c == 0)
  {
    h = 0.0; // Undefined Hue because max == min
    s = 0.0;
  }
  else
  {
    r = static_cast<double>(rgb.red()) / 255.0;
    g = static_cast<double>(rgb.green()) / 255.0;
    b = static_cast<double>(rgb.blue()) / 255.0;
    s = chroma / v;

    if (M == rgb.red())
    {
      hue_prime = (g - b) / chroma;

      while (hue_prime < 0.0)
        hue_prime += 6.0;
      hue_prime = fmod(hue_prime, 6.0);
    }
    else if (M == rgb.green())
    {
      hue_prime = ((b - r) / chroma) + 2.0;
    }
    else if (M == rgb.blue())
    {
      hue_prime = ((r - g) / chroma) + 4.0;
    }

    h = hue_prime * 60.0;
  }

  m_hue = h;
  m_saturation = s;
  m_value = v;
}

int Hsv::hueInt() const
{
  return static_cast<int>(floor(m_hue + 0.5));
}

int Hsv::saturationInt() const
{
  return static_cast<int>(floor(m_saturation * 100.0 + 0.5));
}

int Hsv::valueInt() const
{
  return static_cast<int>(floor(m_value * 100.0 + 0.5));
}

} // namespace gfx
