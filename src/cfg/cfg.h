// Config Library
// Aseprite  | Copyright (C) 2014 David Capello
// Besprited | Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#pragma once

#include <string>

namespace cfg
{

class CfgFile
{
public:
  CfgFile();
  ~CfgFile();

  [[nodiscard]] const std::string& filename() const;

  const char* getValue(const char* section, const char* name,
                       const char* defaultValue) const;
  bool getBoolValue(const char* section, const char* name,
                    bool defaultValue) const;
  int getIntValue(const char* section, const char* name,
                  int defaultValue) const;
  double getDoubleValue(const char* section, const char* name,
                        double defaultValue) const;

  void setValue(const char* section, const char* name, const char* value);
  void setBoolValue(const char* section, const char* name, bool value);
  void setIntValue(const char* section, const char* name, int value);
  void setDoubleValue(const char* section, const char* name, double value);

  void deleteValue(const char* section, const char* name);

  void load(const std::string& filename);

  // Writes the configuration to the file it was loaded from. Returns false
  // if it could not be written completely; lastError() then says why.
  [[nodiscard]] bool save();
  [[nodiscard]] const std::string& lastError() const;

private:
  class CfgFileImpl;
  CfgFileImpl* m_impl;
};

} // namespace cfg
