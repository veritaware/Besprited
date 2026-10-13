// LibreSprite | Copyright (C) 2026      LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.
//
// Script permissions: a script requests access to privileged APIs via
// "///permission <name>" lines in the leading "///" header (the same
// block scanScript() reads for "///listen"). The first run of a script
// asks the user which of the requested permissions to grant; the grants
// are persisted in a JSON file in the user dir. The API extensions call
// check() at call time for the active script.

#pragma once

#include <map>
#include <set>
#include <string>

namespace app
{

class ScriptPermissions
{
public:
  static const char* const kFsRead;    // "fs.read"
  static const char* const kFsWrite;   // "fs.write"
  static const char* const kFetch;     // "fetch"
  static const char* const kAppLaunch; // "app.launch"
  static const char* const kServer;    // "server"

  // True if the name is one of the known permissions.
  static bool isKnownPermission(const std::string& name);

  // Parses the "///permission" lines of a script header (the leading
  // "///" block). Unknown names are ignored.
  static std::set<std::string> parseRequested(const std::string& path);

  // Grants made for this run only (headless auto-grants): not persisted.
  static void setSessionGranted(const std::string& path, const std::set<std::string>& perms);

  // Persisted grant records, keyed by the canonical script path (the
  // same form as AppScripting::getFileName()).
  static std::set<std::string> granted(const std::string& path);
  static void setGranted(const std::string& path, const std::set<std::string>& perms);
  static void removeRecord(const std::string& path);
  static std::map<std::string, std::set<std::string>> allRecords();

  // Active script context: set by AppScripting before each eval; the
  // API extensions call check() at call time. Unrestricted by default
  // (REPL lines and headless evals of non-file code).
  static void setActiveContext(const std::string& path, const std::set<std::string>& perms);
  static void setUnrestricted();
  static bool check(const std::string& perm);

  // Restores the active context when destroyed. Used to run code under a
  // different context (REPL lines, nested scripts, events) without leaking
  // it to whatever runs next.
  class ScopedContext
  {
  public:
    ScopedContext();
    // Switches to the grants of `path` (the persisted record).
    explicit ScopedContext(const std::string& path);
    ~ScopedContext();
    ScopedContext(const ScopedContext&) = delete;
    ScopedContext& operator=(const ScopedContext&) = delete;

  private:
    bool m_unrestricted;
    std::string m_path;
    std::set<std::string> m_perms;
  };

  // Human-readable denial message for error results and the console.
  static std::string denyMessage(const std::string& perm);
};

} // namespace app
