// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/script/script_permissions.h"

#include <filesystem>
#include <fstream>

using app::ScriptPermissions;

namespace
{

// A path with no persisted record, so these tests never touch the user's
// permission store.
const char* kScript = "/nonexistent/besprited-test/script.js";

} // namespace

TEST(ScriptPermissions, ScopedContextRestoresThePreviousContext)
{
  ScriptPermissions::setUnrestricted();
  EXPECT_TRUE(ScriptPermissions::check(ScriptPermissions::kFsWrite));
  {
    ScriptPermissions::ScopedContext scope(kScript);
    EXPECT_FALSE(ScriptPermissions::check(ScriptPermissions::kFsWrite));
    EXPECT_FALSE(ScriptPermissions::check(ScriptPermissions::kServer));
  }
  EXPECT_TRUE(ScriptPermissions::check(ScriptPermissions::kFsWrite));
}

TEST(ScriptPermissions, NestedScopesRestoreTheCallersGrants)
{
  ScriptPermissions::setActiveContext("a", {ScriptPermissions::kFetch});
  {
    ScriptPermissions::ScopedContext inner(kScript);
    ScriptPermissions::setActiveContext("b", {ScriptPermissions::kFsRead});
    EXPECT_TRUE(ScriptPermissions::check(ScriptPermissions::kFsRead));
    EXPECT_FALSE(ScriptPermissions::check(ScriptPermissions::kFetch));
  }
  EXPECT_TRUE(ScriptPermissions::check(ScriptPermissions::kFetch));
  EXPECT_FALSE(ScriptPermissions::check(ScriptPermissions::kFsRead));
  ScriptPermissions::setUnrestricted();
}

TEST(ScriptPermissions, SessionGrantsAreVisibleButNotPersisted)
{
  ScriptPermissions::setSessionGranted(kScript, {ScriptPermissions::kServer});
  EXPECT_EQ(1u, ScriptPermissions::granted(kScript).count(ScriptPermissions::kServer));
  EXPECT_EQ(0u, ScriptPermissions::allRecords().count(kScript));
  {
    ScriptPermissions::ScopedContext scope(kScript);
    EXPECT_TRUE(ScriptPermissions::check(ScriptPermissions::kServer));
    EXPECT_FALSE(ScriptPermissions::check(ScriptPermissions::kFsRead));
  }
  ScriptPermissions::setSessionGranted(kScript, {});
}

TEST(ScriptPermissions, ParseRequestedReadsTheLeadingHeaderOnly)
{
  auto path = std::filesystem::temp_directory_path() / "besprited_perm_parse_test.js";
  {
    std::ofstream file{path};
    file << "///permission server fs.read bogus\n"
            "///permission fetch\n"
            "// not a header line\n"
            "///permission app.launch\n";
  }
  auto requested = ScriptPermissions::parseRequested(path.string());
  std::filesystem::remove(path);

  EXPECT_EQ(3u, requested.size());
  EXPECT_EQ(1u, requested.count(ScriptPermissions::kServer));
  EXPECT_EQ(1u, requested.count(ScriptPermissions::kFsRead));
  EXPECT_EQ(1u, requested.count(ScriptPermissions::kFetch));
  EXPECT_EQ(0u, requested.count(ScriptPermissions::kAppLaunch));
}
