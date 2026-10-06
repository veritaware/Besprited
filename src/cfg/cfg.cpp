// Config Library
// Aseprite  | Copyright (C) 2014-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "cfg/cfg.h"
#include "cfg/cfg_wasm.h"

#include "base/file_handle.h"
#include "base/log.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "SimpleIni.h"

namespace cfg
{

class CfgFile::CfgFileImpl
{
public:
  const std::string& filename() const { return m_filename; }

  const char* getValue(const char* section, const char* name, const char* defaultValue) const
  {
    return m_ini.GetValue(section, name, defaultValue);
  }

  bool getBoolValue(const char* section, const char* name, bool defaultValue) const
  {
    return m_ini.GetBoolValue(section, name, defaultValue);
  }

  int getIntValue(const char* section, const char* name, int defaultValue) const
  {
    return static_cast<int>(m_ini.GetLongValue(section, name, defaultValue));
  }

  double getDoubleValue(const char* section, const char* name, double defaultValue) const
  {
    return m_ini.GetDoubleValue(section, name, defaultValue);
  }

  void setValue(const char* section, const char* name, const char* value) { m_ini.SetValue(section, name, value); }

  void setBoolValue(const char* section, const char* name, bool value) { m_ini.SetBoolValue(section, name, value); }

  void setIntValue(const char* section, const char* name, int value) { m_ini.SetLongValue(section, name, value); }

  void setDoubleValue(const char* section, const char* name, double value)
  {
    m_ini.SetDoubleValue(section, name, value);
  }

  void deleteValue(const char* section, const char* name) { m_ini.Delete(section, name, true); }

  void load(const std::string& filename)
  {
    m_filename = filename;

    const base::FileHandle file(base::open_file(m_filename, "rb"));
    if (file)
    {
      if (const SI_Error err = m_ini.LoadFile(file.get()); err != SI_OK)
        LOG("Error '%d' loading configuration from '%s'.", err, m_filename.c_str());
    }
    else
    {
      std::string data;
#ifdef __EMSCRIPTEN__
      data = cfg_wasm_load(m_filename);
#endif
      if (!data.empty())
        m_ini.LoadData(data);
    }
  }

  bool save()
  {
    m_lastError.clear();

    std::string data;
    if (const SI_Error err = m_ini.Save(data); err != SI_OK)
    {
      LOG("Error '%d' saving configuration into '%s'.", err, m_filename.c_str());
      m_lastError = "could not serialize the configuration";
      return false;
    }

#ifdef __EMSCRIPTEN__
    cfg_wasm_save(m_filename, data);
#endif

#ifdef __EMSCRIPTEN__
    // The browser storage above is the source of truth there.
    writeFile(data);
    return true;
#else
    return writeFile(data);
#endif
  }

  bool writeFile(const std::string& data)
  {
    const base::FileHandle file(base::open_file(m_filename, "wb"));
    if (!file)
    {
      m_lastError = std::strerror(errno);
      LOG("Error opening '%s' to save the configuration: %s.", m_filename.c_str(), m_lastError.c_str());
      return false;
    }

    // Check for short writes and for errors that are only reported on
    // flush (disk full, quota exceeded, ...).
    if (std::fwrite(data.c_str(), 1, data.size(), file.get()) != data.size() || std::fflush(file.get()) != 0)
    {
      m_lastError = std::strerror(errno);
      LOG("Error writing the configuration into '%s': %s.", m_filename.c_str(), m_lastError.c_str());
      return false;
    }
    return true;
  }

  const std::string& lastError() const { return m_lastError; }

private:
  std::string m_filename;
  std::string m_lastError;
  CSimpleIniA m_ini;
};

CfgFile::CfgFile()
  : m_impl(new CfgFileImpl)
{
}

CfgFile::~CfgFile()
{
  delete m_impl;
}

const std::string& CfgFile::filename() const
{
  return m_impl->filename();
}

const char* CfgFile::getValue(const char* section, const char* name, const char* defaultValue) const
{
  return m_impl->getValue(section, name, defaultValue);
}

bool CfgFile::getBoolValue(const char* section, const char* name, bool defaultValue) const
{
  return m_impl->getBoolValue(section, name, defaultValue);
}

int CfgFile::getIntValue(const char* section, const char* name, int defaultValue) const
{
  return m_impl->getIntValue(section, name, defaultValue);
}

double CfgFile::getDoubleValue(const char* section, const char* name, double defaultValue) const
{
  return m_impl->getDoubleValue(section, name, defaultValue);
}

void CfgFile::setValue(const char* section, const char* name, const char* value)
{
  m_impl->setValue(section, name, value);
}

void CfgFile::setBoolValue(const char* section, const char* name, bool value)
{
  m_impl->setBoolValue(section, name, value);
}

void CfgFile::setIntValue(const char* section, const char* name, int value)
{
  m_impl->setIntValue(section, name, value);
}

void CfgFile::setDoubleValue(const char* section, const char* name, double value)
{
  m_impl->setDoubleValue(section, name, value);
}

void CfgFile::deleteValue(const char* section, const char* name)
{
  m_impl->deleteValue(section, name);
}

void CfgFile::load(const std::string& filename)
{
  m_impl->load(filename);
}

bool CfgFile::save()
{
  return m_impl->save();
}

const std::string& CfgFile::lastError() const
{
  return m_impl->lastError();
}

} // namespace cfg
