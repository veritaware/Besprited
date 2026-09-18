// Network Library
// Aseprite  | Copyright (C) 2001-2015 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "net/http_response.h"

#include <ostream>

namespace net
{

void HttpResponse::write(const char* data, std::size_t length)
{
  m_stream->write(data, static_cast<std::streamsize>(length));
}

} // namespace net
