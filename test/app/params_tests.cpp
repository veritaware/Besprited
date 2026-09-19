// Besprited | Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#include "tests.h"

#include "app/commands/params.h"

using namespace app;

TEST(Params, GetOnMissingKeyDoesNotInsertAnEntry)
{
  Params p;
  EXPECT_EQ("", p.get("missing"));
  EXPECT_TRUE(p.empty());
  EXPECT_FALSE(p.has_param("missing"));
}

TEST(Params, GetOnExistingKeyReturnsTheValue)
{
  Params p;
  p.set("key", "value");
  EXPECT_EQ("value", p.get("key"));
}

TEST(Params, CopyStaysEqualAfterReadingAbsentKeysFromTheCopy)
{
  Params original;
  original.set("key", "value");

  Params copy = original;
  // Reading a key that isn't there must not grow the map (this used to
  // silently insert via operator[], corrupting Params instances held as
  // persistent state elsewhere - see issue #219).
  copy.get("missing");
  copy.get_as<int>("also_missing");

  EXPECT_TRUE(original == copy);
}

TEST(Params, GetAsBoolParsesTrueFalseAndNumericForms)
{
  Params p;
  p.set("a", "true");
  p.set("b", "false");
  p.set("c", "1");
  p.set("d", "0");
  p.set("e", "garbage");

  EXPECT_TRUE(p.get_as<bool>("a"));
  EXPECT_FALSE(p.get_as<bool>("b"));
  EXPECT_TRUE(p.get_as<bool>("c"));
  EXPECT_FALSE(p.get_as<bool>("d"));
  EXPECT_FALSE(p.get_as<bool>("e"));
  EXPECT_FALSE(p.get_as<bool>("missing"));
}

TEST(Params, GetAsIntParsesOrDefaultsOnMissingKey)
{
  Params p;
  p.set("n", "42");
  EXPECT_EQ(42, p.get_as<int>("n"));
  EXPECT_EQ(0, p.get_as<int>("missing"));
}

TEST(Params, OperatorOrEqualsMergesIntoANonConstInstance)
{
  Params base;
  base.set("a", "1");

  Params extra;
  extra.set("b", "2");

  base |= extra;
  EXPECT_EQ("1", base.get("a"));
  EXPECT_EQ("2", base.get("b"));
}
