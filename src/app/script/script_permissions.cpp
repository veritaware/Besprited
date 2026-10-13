// LibreSprite | Copyright (C) 2026      LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "app/script/script_permissions.h"

#include "app/resource_finder.h"
#include "base/string.h"
#include "base/trim_string.h"
#include "delta/JSON.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <system_error>

namespace app
{

const char* const ScriptPermissions::kFsRead = "fs.read";
const char* const ScriptPermissions::kFsWrite = "fs.write";
const char* const ScriptPermissions::kFetch = "fetch";
const char* const ScriptPermissions::kAppLaunch = "app.launch";
const char* const ScriptPermissions::kServer = "server";

namespace
{

bool isKnown(const std::string& name)
{
  return name == ScriptPermissions::kFsRead || name == ScriptPermissions::kFsWrite ||
         name == ScriptPermissions::kFetch || name == ScriptPermissions::kAppLaunch ||
         name == ScriptPermissions::kServer;
}

// The on-disk path of the grant records (user dir).
std::string storePath()
{
  ResourceFinder rf{false};
  rf.includeUserDir("script_permissions.json");
  return rf.defaultFilename();
}

// The in-memory grant records (loaded from the store file on first
// use; only the main thread touches them).
std::map<std::string, std::set<std::string>>& records()
{
  static std::map<std::string, std::set<std::string>> store;
  static bool loaded = false;
  if (!loaded)
  {
    loaded = true;
    std::ifstream file{storePath(), std::ios::binary};
    if (!file)
      return store;
    auto source = std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    JSON::Value root;
    try
    {
      root = JSON::parse(source);
    }
    catch (const std::exception&)
    {
      return store; // Corrupt file: start with an empty store
    }
    if (!root.isObject())
      return store;
    for (auto& [path, value] : root.object().map)
    {
      if (!value.isObject())
        continue;
      auto it = value.object().find("granted");
      if (it == value.object().end() || !it->second.isArray())
        continue;
      std::set<std::string> perms;
      for (auto& entry : it->second.array())
        if (entry.isString() && isKnown(entry.string()))
          perms.insert(entry.string());
      store[path] = std::move(perms);
    }
  }
  return store;
}

void saveStore(const std::map<std::string, std::set<std::string>>& store)
{
  JSON::Value root;
  root.object();
  for (auto& [path, perms] : store)
  {
    JSON::Value entry;
    entry.object();
    for (auto& perm : perms)
      entry["granted"].array().push_back(perm);
    root[path] = entry;
  }
  auto path = storePath();
  std::error_code ec;
  std::filesystem::create_directories(std::filesystem::path(path).parent_path(), ec);
  std::ofstream file{path, std::ios::binary | std::ios::trunc};
  if (!file)
    return;
  file << root.toJSON(1);
}

// The permissions of the currently running script (set by
// AppScripting before each eval); unrestricted by default.
bool unrestricted = true;
std::string activePath;
std::set<std::string> activePerms;

// Grants that live only for this process (headless auto-grants).
std::map<std::string, std::set<std::string>>& sessionRecords()
{
  static std::map<std::string, std::set<std::string>> s;
  return s;
}

void refreshActive(const std::string& path, const std::set<std::string>& perms)
{
  if (!unrestricted && path == activePath)
    activePerms = perms;
}

} // anonymous namespace

bool ScriptPermissions::isKnownPermission(const std::string& name)
{
  return isKnown(name);
}

std::set<std::string> ScriptPermissions::parseRequested(const std::string& path)
{
  std::set<std::string> result;
  std::ifstream file{path};
  if (!file)
    return result;
  std::string line;
  while (std::getline(file, line))
  {
    base::trim_string(line, line);
    auto parts = base::split(line, ' ');
    for (auto& part : parts)
      base::trim_string(part, part);
    if (parts.empty() || parts[0].size() < 3 || parts[0].substr(0, 3) != "///")
      break;
    base::trim_string(parts[0].substr(3), parts[0]);
    if (parts[0].empty())
    {
      parts.erase(parts.begin());
      if (parts.empty())
        break;
    }
    auto key = base::string_to_lower(parts[0]);
    parts.erase(parts.begin());
    if (key != "permission")
      continue;
    for (auto& perm : parts)
      if (isKnown(perm))
        result.insert(perm);
  }
  return result;
}

std::set<std::string> ScriptPermissions::granted(const std::string& path)
{
  std::set<std::string> result;
  auto& store = records();
  if (auto it = store.find(path); it != store.end())
    result = it->second;
  auto& session = sessionRecords();
  if (auto it = session.find(path); it != session.end())
    result.insert(it->second.begin(), it->second.end());
  return result;
}

void ScriptPermissions::setSessionGranted(const std::string& path, const std::set<std::string>& perms)
{
  sessionRecords()[path] = perms;
}

void ScriptPermissions::setGranted(const std::string& path, const std::set<std::string>& perms)
{
  records()[path] = perms;
  sessionRecords().erase(path);
  saveStore(records());
  refreshActive(path, perms);
}

void ScriptPermissions::removeRecord(const std::string& path)
{
  records().erase(path);
  sessionRecords().erase(path);
  saveStore(records());
  refreshActive(path, {});
}

std::map<std::string, std::set<std::string>> ScriptPermissions::allRecords()
{
  return records();
}

void ScriptPermissions::setActiveContext(const std::string& path, const std::set<std::string>& perms)
{
  activePath = path;
  activePerms = perms;
  unrestricted = false;
}

void ScriptPermissions::setUnrestricted()
{
  unrestricted = true;
}

ScriptPermissions::ScopedContext::ScopedContext()
  : m_unrestricted(unrestricted)
  , m_path(activePath)
  , m_perms(activePerms)
{
}

ScriptPermissions::ScopedContext::ScopedContext(const std::string& path)
  : ScopedContext()
{
  setActiveContext(path, granted(path));
}

ScriptPermissions::ScopedContext::~ScopedContext()
{
  unrestricted = m_unrestricted;
  activePath = m_path;
  activePerms = m_perms;
}

bool ScriptPermissions::check(const std::string& perm)
{
  return unrestricted || activePerms.count(perm) > 0;
}

std::string ScriptPermissions::denyMessage(const std::string& perm)
{
  return "Permission denied: '" + perm + "' permission not granted to this script";
}

} // namespace app
