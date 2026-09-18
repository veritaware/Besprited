/*
 * Fixed point type.
 * Based on Allegro library by Shawn Hargreaves.
 */
// Copyright (C) 2026 Veritaware

#pragma once

#include "base/ints.h"
#include <cerrno>

namespace fixmath
{

using fixed = int32_t;

extern const fixed fixtorad_r;
extern const fixed radtofix_r;

extern fixed _cos_tbl[];
extern fixed _tan_tbl[];
extern fixed _acos_tbl[];

fixed fixsqrt(fixed x);
fixed fixhypot(fixed x, fixed y);
fixed fixatan(fixed x);
fixed fixatan2(fixed y, fixed x);

// ftofix and fixtof are used in generic C versions of fixmul and fixdiv
inline fixed ftofix(const double x)
{
  if (x > 32767.0)
  {
    errno = ERANGE;
    return 0x7FFFFFFF;
  }

  if (x < -32767.0)
  {
    errno = ERANGE;
    return -0x7FFFFFFF;
  }

  return static_cast<fixed>(x * 65536.0 + (x < 0 ? -0.5 : 0.5));
}

inline double fixtof(const fixed x)
{
  return static_cast<double>(x) / 65536.0;
}

inline fixed fixadd(const fixed x, const fixed y)
{
  const fixed result = x + y;

  if (result >= 0)
  {
    if ((x < 0) && (y < 0))
    {
      errno = ERANGE;
      return -0x7FFFFFFF;
    }
    return result;
  }
  else
  {
    if ((x > 0) && (y > 0))
    {
      errno = ERANGE;
      return 0x7FFFFFFF;
    }
    return result;
  }
}

inline fixed fixsub(const fixed x, const fixed y)
{
  const fixed result = x - y;

  if (result >= 0)
  {
    if ((x < 0) && (y > 0))
    {
      errno = ERANGE;
      return -0x7FFFFFFF;
    }
    return result;
  }
  else
  {
    if ((x > 0) && (y < 0))
    {
      errno = ERANGE;
      return 0x7FFFFFFF;
    }
    return result;
  }
}

inline fixed fixmul(const fixed x, const fixed y)
{
  return ftofix(fixtof(x) * fixtof(y));
}

inline fixed fixdiv(const fixed x, const fixed y)
{
  if (y == 0)
  {
    errno = ERANGE;
    return (x < 0) ? -0x7FFFFFFF : 0x7FFFFFFF;
  }
  return ftofix(fixtof(x) / fixtof(y));
}

inline int fixfloor(const fixed x)
{
  /* (x >> 16) is not portable */
  if (x >= 0)
    return x >> 16;
  else
    return ~(~x >> 16);
}

inline int fixceil(const fixed x)
{
  if (x > 0x7FFF0000)
  {
    errno = ERANGE;
    return 0x7FFF;
  }

  return fixfloor(x + 0xFFFF);
}

inline fixed itofix(const int x)
{
  return x << 16;
}

inline int fixtoi(const fixed x)
{
  return fixfloor(x) + ((x & 0x8000) >> 15);
}

inline fixed fixcos(const fixed x)
{
  return _cos_tbl[((x + 0x4000) >> 15) & 0x1FF];
}

inline fixed fixsin(const fixed x)
{
  return _cos_tbl[((x - 0x400000 + 0x4000) >> 15) & 0x1FF];
}

inline fixed fixtan(const fixed x)
{
  return _tan_tbl[((x + 0x4000) >> 15) & 0xFF];
}

inline fixed fixacos(const fixed x)
{
  if ((x < -65536) || (x > 65536))
  {
    errno = EDOM;
    return 0;
  }

  return _acos_tbl[(x + 65536 + 127) >> 8];
}

inline fixed fixasin(const fixed x)
{
  if ((x < -65536) || (x > 65536))
  {
    errno = EDOM;
    return 0;
  }

  return 0x00400000 - _acos_tbl[(x + 65536 + 127) >> 8];
}

} // namespace fixmath
