// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

// Coverage for delta's JSON::Value - the variant that carries every value
// crossing the C++ <-> JS boundary (script::Value is an alias of it).

#include <gtest/gtest.h>

#include "script/value.h"

#include <memory>
#include <string>
#include <vector>

using script::Value;

TEST(JsonValue, DefaultIsUndefined)
{
  Value v;
  EXPECT_TRUE(v.isUndefined());
  EXPECT_FALSE(v.isNull());
  EXPECT_EQ("Undefined", v.type());
  EXPECT_EQ("undefined", v.toString());
}

TEST(JsonValue, SpecialsAreDistinct)
{
  Value null{JSON::Special::Null};
  EXPECT_TRUE(null.isNull());
  EXPECT_FALSE(null.isUndefined());
  EXPECT_EQ("null", null.toString());
}

TEST(JsonValue, NumberRoundTripAndConversions)
{
  Value v{3.75};
  EXPECT_TRUE(v.isNumber());
  EXPECT_DOUBLE_EQ(3.75, v.number());
  EXPECT_DOUBLE_EQ(3.75, static_cast<double>(v));
  EXPECT_EQ(3, static_cast<int>(v)); // truncates
  EXPECT_EQ(3, static_cast<int64_t>(v));
  EXPECT_FLOAT_EQ(3.75f, static_cast<float>(v));

  Value notANumber{"abc"};
  EXPECT_DOUBLE_EQ(0.0, static_cast<double>(notANumber));
  EXPECT_DOUBLE_EQ(1.5, notANumber.number(1.5)); // default for the wrong type
}

TEST(JsonValue, StringRoundTripAndConversions)
{
  Value fromLiteral{"hello"};
  EXPECT_TRUE(fromLiteral.isString());
  EXPECT_FALSE(fromLiteral.isBinString());
  EXPECT_EQ("hello", fromLiteral.string());
  EXPECT_EQ("hello", static_cast<const std::string&>(fromLiteral));
  EXPECT_EQ("String", fromLiteral.type());

  Value fromStd{std::string{"world"}};
  EXPECT_EQ("world", fromStd.string());

  Value number{5.0};
  EXPECT_EQ("fallback", number.string("fallback"));
}

TEST(JsonValue, BinaryStringsAreFlagged)
{
  Value bin{Value::String{std::string{"\xff\x00", 2}, true}};
  EXPECT_TRUE(bin.isString());
  EXPECT_TRUE(bin.isBinString());
  EXPECT_EQ(2u, bin.string().size());

  Value copy = bin;
  EXPECT_TRUE(copy.isBinString());
}

TEST(JsonValue, BooleanRoundTrip)
{
  Value t{true};
  EXPECT_TRUE(t.isBoolean());
  EXPECT_TRUE(t.boolean());
  EXPECT_TRUE(static_cast<bool>(t));
  EXPECT_EQ("true", t.toString());

  Value s{"nope"};
  EXPECT_FALSE(static_cast<bool>(s));
  EXPECT_TRUE(s.boolean(true));
}

TEST(JsonValue, IndexingByKeyTurnsAnUndefinedValueIntoAnObject)
{
  Value v;
  v["a"] = 1.0;
  v["b"] = "two";

  EXPECT_TRUE(v.isObject());
  EXPECT_EQ(2u, v.object().size());
  EXPECT_TRUE(v.object().contains("a"));
  EXPECT_DOUBLE_EQ(1.0, v["a"].number());
  EXPECT_EQ("two", v["b"].string());
  EXPECT_EQ("Object", v.type());
}

TEST(JsonValue, IndexingByPositionTurnsAnUndefinedValueIntoAnArray)
{
  Value v;
  v[2] = "third";

  ASSERT_TRUE(v.isArray());
  ASSERT_EQ(3u, v.array().size());
  EXPECT_TRUE(v[0].isUndefined());
  EXPECT_EQ("third", v[2].string());

  v.push_back(4.0);
  EXPECT_EQ(4u, v.array().size());
  EXPECT_DOUBLE_EQ(4.0, v[3].number());
}

TEST(JsonValue, ObjectsAndArraysAreSharedByReferenceOnCopy)
{
  Value a;
  a["x"] = 1.0;
  Value b = a; // shares the same ObjectRef
  b["x"] = 2.0;
  EXPECT_DOUBLE_EQ(2.0, a["x"].number());

  Value arr;
  arr.push_back(1.0);
  Value arr2 = arr;
  arr2.push_back(2.0);
  EXPECT_EQ(2u, arr.array().size());
}

TEST(JsonValue, ByteArrayRoundTrip)
{
  auto bytes = std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>{1, 2, 3});
  Value v{bytes};
  EXPECT_TRUE(v.isByteArray());
  EXPECT_EQ("ByteArray", v.type());
  ASSERT_EQ(3u, v.byteArray().size());
  EXPECT_EQ(2, v.byteArray()[1]);
  EXPECT_EQ(bytes.get(), &v.byteArray()); // shared, not copied

  Value other{5.0};
  EXPECT_TRUE(other.byteArray().empty()); // accessor coerces to an empty array
  EXPECT_TRUE(other.isByteArray());
}

TEST(JsonValue, NativeRefRoundTripsTypedPointers)
{
  struct Thing { int n = 7; };
  auto thing = std::make_shared<Thing>();

  Value v = JSON::makeNative(thing);
  ASSERT_TRUE(v.isNative());
  EXPECT_EQ("NativeRef", v.type());
  EXPECT_EQ(thing.get(), v.asNative<Thing>().get());
  EXPECT_EQ(7, static_cast<std::shared_ptr<Thing>>(v)->n);

  struct Other {};
  EXPECT_EQ(nullptr, v.asNative<Other>()); // type mismatch -> null

  std::shared_ptr<Thing> none;
  EXPECT_TRUE(JSON::makeNative(none).isNull());
}

TEST(JsonValue, FunctionsAreInvocable)
{
  Value fn{JSON::FunctionRef{[](const JSON::Array& args) -> Value {
    return args.empty() ? Value{JSON::Special::Undefined}
                        : Value{static_cast<double>(args.size()) + static_cast<double>(args[0])};
  }, std::any{}}};
  ASSERT_TRUE(fn.isFunction());
  EXPECT_EQ("Function", fn.type());

  JSON::Array args;
  args.push_back(10.0);
  args.push_back(0.0);
  EXPECT_DOUBLE_EQ(12.0, fn(args).number());

  Value notAFunction{1.0};
  EXPECT_TRUE(notAFunction(args).isUndefined());
}

TEST(JsonValue, SerializesToJsonAndParsesBack)
{
  Value v;
  v["name"] = "bes\"prited";
  v["n"] = 42.0;
  v["ok"] = true;
  v["nothing"] = JSON::Special::Null;
  v["list"].push_back(1.0);
  v["list"].push_back("two");

  Value back = JSON::parse(v.toJSON());

  ASSERT_TRUE(back.isObject());
  EXPECT_EQ("bes\"prited", back["name"].string());
  EXPECT_DOUBLE_EQ(42.0, back["n"].number());
  EXPECT_TRUE(back["ok"].boolean());
  EXPECT_TRUE(back["nothing"].isNull());
  ASSERT_TRUE(back["list"].isArray());
  EXPECT_DOUBLE_EQ(1.0, back["list"][0].number());
  EXPECT_EQ("two", back["list"][1].string());
}

TEST(JsonValue, ParsesNestedDocuments)
{
  Value v = JSON::parse(R"({"a": [1, 2, {"b": "c"}], "d": {"e": null, "f": false}})");
  ASSERT_TRUE(v.isObject());
  EXPECT_DOUBLE_EQ(2.0, v["a"][1].number());
  EXPECT_EQ("c", v["a"][2]["b"].string());
  EXPECT_TRUE(v["d"]["e"].isNull());
  EXPECT_FALSE(v["d"]["f"].boolean());
}

TEST(JsonValue, FunctionsAndUndefinedAreSkippedWhenSerializing)
{
  Value v;
  v["fn"] = JSON::FunctionRef{[](const JSON::Array&) -> Value { return {}; }, std::any{}};
  v["u"] = JSON::Special::Undefined;
  v["kept"] = 1.0;

  EXPECT_EQ(std::string::npos, v.toJSON().find("fn"));
  EXPECT_NE(std::string::npos, v.toJSON().find("undefined"));
  EXPECT_EQ(std::string::npos, v.toJSON(0, JSON::NO_UNDEFINED).find("\"u\""));
  EXPECT_NE(std::string::npos, v.toJSON(0, JSON::NO_UNDEFINED).find("kept"));
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
