// Base Library
// Aseprite  | Copyright (C) 2001-2013 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <exception>
#include <string>

namespace base
{

class Exception : public std::exception
{
public:
  Exception() noexcept;
  Exception(const char* format, ...) noexcept;
  Exception(const std::string& msg) noexcept;
  ~Exception() noexcept override;

  [[nodiscard]] const char* what() const noexcept override;

protected:
  void setMessage(const char* msg) noexcept;

private:
  std::string m_msg;
};

} // namespace base
