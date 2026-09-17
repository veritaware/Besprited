// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

// Coverage for the delta `Interpreter` (third_party/delta) as backed by the
// quickjs-ng submodule: script/module evaluation, the C++ <-> JS value
// bridge, API functions, native class bindings and the event loop tick.

#include <gtest/gtest.h>

#include "delta/Interpreter.hpp"
#include "delta/JSON.hpp"
#include "di.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>
#include <vector>

namespace
{

struct Thing
{
  int value = 0;
  static inline int liveCount = 0;
  Thing() { ++liveCount; }
  ~Thing() { --liveCount; }
};

class InterpreterTest : public ::testing::Test
{
protected:
  std::shared_ptr<Interpreter> engine;
  // Whatever JS last handed to the native "capture" API function.
  JSON::Value captured;

  void SetUp() override
  {
    engine = di::inject<Interpreter>("js");
    ASSERT_TRUE(engine);
    engine->addAPIFunction("capture",
                           [this](JSON::Array& args) -> JSON::Value
                           {
                             captured =
                                 args.empty()
                                     ? JSON::Value{JSON::Special::Undefined}
                                     : args[0];
                             return JSON::Value{JSON::Special::Undefined};
                           });
  }

  JSON::Value run(const std::string& code)
  {
    return engine->eval(code, "test", Interpreter::EvalType::Script);
  }

  // Interpreter::ClassDef keeps a *reference* to the constructor callable
  // (the app's Extension objects own theirs), so it must outlive the
  // interpreter - hence a fixture member rather than a temporary.
  Interpreter::ClassCtor thingCtor =
      [](JSON::Array& args) -> std::shared_ptr<void>
  {
    auto t = std::make_shared<Thing>();
    if (!args.empty())
      t->value = static_cast<int>(args[0].number());
    return t;
  };

  Interpreter::ClassDef& addThingClass(Interpreter& target)
  {
    return target.addClass("Thing", typeid(void), typeid(Thing), thingCtor);
  }
};

} // namespace

TEST_F(InterpreterTest, ScriptEvalReturnsTheCompletionValue)
{
  JSON::Value v = run("1 + 1;");
  ASSERT_TRUE(v.isNumber());
  EXPECT_DOUBLE_EQ(2.0, v.number());

  JSON::Value s = run("'a' + 'b';");
  ASSERT_TRUE(s.isString());
  EXPECT_EQ("ab", s.string());

  EXPECT_TRUE(run("undefined;").isUndefined());
  EXPECT_TRUE(run("null;").isNull());
  EXPECT_TRUE(run("true;").isBoolean());
}

TEST_F(InterpreterTest, TopLevelVarPersistsAcrossScriptEvals)
{
  run("var counter = 41;");
  JSON::Value v = run("counter + 1;");
  EXPECT_DOUBLE_EQ(42.0, v.number());
}

TEST_F(InterpreterTest, SyntaxErrorThrowsARuntimeError)
{
  EXPECT_THROW(run("this is not valid javascript {{{"), std::runtime_error);
}

TEST_F(InterpreterTest, ThrowingScriptSurfacesTheMessageAndStack)
{
  try
  {
    run("function inner() { throw new Error('boom'); }\ninner();");
    FAIL() << "expected an exception";
  }
  catch (const std::runtime_error& e)
  {
    std::string what = e.what();
    EXPECT_NE(std::string::npos, what.find("boom"));
    EXPECT_NE(std::string::npos, what.find("inner"))
        << "stack trace should name the throwing function";
  }
}

TEST_F(InterpreterTest, EngineKeepsWorkingAfterAFailedEval)
{
  EXPECT_THROW(run("throw 1;"), std::runtime_error);
  EXPECT_DOUBLE_EQ(3.0, run("3;").number());
}

TEST_F(InterpreterTest, ModuleEvalRejectsOnATopLevelThrow)
{
  EXPECT_THROW(
      engine->eval("export const x = 1; throw new Error('module boom');",
                   "mod.js", Interpreter::EvalType::Module),
      std::runtime_error);
  // ...and still runs a good module afterwards.
  EXPECT_NO_THROW(engine->eval("capture('from module');", "ok.js",
                               Interpreter::EvalType::Module));
  EXPECT_EQ("from module", captured.string());
}

TEST_F(InterpreterTest, BareImportSpecifiersResolveThroughModuleSearchPaths)
{
  auto dir =
      std::filesystem::path(::testing::TempDir()) / "delta_interpreter_tests";
  std::filesystem::create_directories(dir);
  {
    std::ofstream out(dir / "answer.js");
    out << "export const answer = 42;\n";
  }
  engine->addModuleSearchPath(dir.string());

  ASSERT_NO_THROW(
      engine->eval("import { answer } from 'answer.js'; capture(answer);",
                   (dir / "main.js").string(), Interpreter::EvalType::Module));
  EXPECT_DOUBLE_EQ(42.0, captured.number());

  // Extension-less specifiers get ".js" appended.
  ASSERT_NO_THROW(
      engine->eval("import { answer } from 'answer'; capture(answer + 1);",
                   (dir / "main2.js").string(), Interpreter::EvalType::Module));
  EXPECT_DOUBLE_EQ(43.0, captured.number());

  std::filesystem::remove_all(dir);
}

TEST_F(InterpreterTest, UnresolvableImportIsAnError)
{
  EXPECT_THROW(engine->eval("import { x } from 'does-not-exist.js';", "main.js",
                            Interpreter::EvalType::Module),
               std::runtime_error);
}

TEST_F(InterpreterTest, ApiFunctionReceivesHeterogeneousArguments)
{
  std::vector<JSON::Value> got;
  engine->addAPIFunction("take",
                         [&](JSON::Array& args) -> JSON::Value
                         {
                           got = args;
                           return JSON::Value{JSON::Special::Undefined};
                         });

  run("take(42, 'hi', 3.5, true, null, undefined);");

  ASSERT_EQ(6u, got.size());
  EXPECT_DOUBLE_EQ(42.0, got[0].number());
  EXPECT_EQ("hi", got[1].string());
  EXPECT_DOUBLE_EQ(3.5, got[2].number());
  EXPECT_TRUE(got[3].boolean());
  EXPECT_TRUE(got[4].isNull());
  EXPECT_TRUE(got[5].isUndefined());
}

TEST_F(InterpreterTest, ApiFunctionReturnValuesReachJs)
{
  engine->addAPIFunction("give",
                         [](JSON::Array&) -> JSON::Value
                         {
                           JSON::Value obj;
                           obj["n"] = 7.0;
                           obj["s"] = "seven";
                           obj["list"].push_back(1.0);
                           obj["list"].push_back(2.0);
                           return obj;
                         });

  JSON::Value v = run("var o = give(); o.n + o.list.length + o.s.length;");
  EXPECT_DOUBLE_EQ(7.0 + 2.0 + 5.0, v.number());
}

TEST_F(InterpreterTest, ArraysAndObjectsRoundTripFromJs)
{
  run("capture({a: 1, b: 'two', c: [3, 4, {d: 5}]});");

  ASSERT_TRUE(captured.isObject());
  EXPECT_DOUBLE_EQ(1.0, captured["a"].number());
  EXPECT_EQ("two", captured["b"].string());
  ASSERT_TRUE(captured["c"].isArray());
  ASSERT_EQ(3u, captured["c"].array().size());
  EXPECT_DOUBLE_EQ(4.0, captured["c"][1].number());
  EXPECT_DOUBLE_EQ(5.0, captured["c"][2]["d"].number());
}

TEST_F(InterpreterTest, Uint8ArrayRoundTripsAsAByteArray)
{
  run("capture(new Uint8Array([1, 2, 255]));");
  ASSERT_TRUE(captured.isByteArray());
  ASSERT_EQ(3u, captured.byteArray().size());
  EXPECT_EQ(1, captured.byteArray()[0]);
  EXPECT_EQ(255, captured.byteArray()[2]);

  auto bytes =
      std::make_shared<std::vector<uint8_t>>(std::vector<uint8_t>{9, 8, 7});
  JSON::Value v{bytes};
  engine->addGlobalValue("bytes", v);
  JSON::Value back = run("bytes instanceof Uint8Array && bytes.length === 3 && "
                         "bytes[0] === 9 && bytes[2] === 7;");
  EXPECT_TRUE(back.boolean());
}

TEST_F(InterpreterTest, BinaryStringsMapEveryByteToOneCharCode)
{
  // A "bin" string is raw bytes, not UTF-8 - each byte must become exactly
  // one char code (this is what LibreSprite's quickjs-amalgam patch
  // JS_NewStringLenBin() provides and what delta emulates on quickjs-ng).
  std::string raw{"\xff\x00\x41\x80", 4};
  JSON::Value bin{JSON::Value::String{raw, true}};
  engine->addGlobalValue("bin", bin);
  JSON::Value v = run("[bin.length, bin.charCodeAt(0), bin.charCodeAt(1), "
                      "bin.charCodeAt(2), bin.charCodeAt(3)].join(',');");
  EXPECT_EQ("4,255,0,65,128", v.string());

  // ...whereas a regular string is decoded as UTF-8.
  JSON::Value utf8{std::string{"\xc3\xa9"}}; // "é"
  engine->addGlobalValue("utf8", utf8);
  JSON::Value w = run("[utf8.length, utf8.charCodeAt(0)].join(',');");
  EXPECT_EQ("1,233", w.string());
}

TEST_F(InterpreterTest, JsFunctionsPassedToNativeAreCallable)
{
  run("capture(function(a, b) { return a * b; });");
  ASSERT_TRUE(captured.isFunction());

  JSON::Array args;
  args.emplace_back(6.0);
  args.emplace_back(7.0);
  JSON::Value result = captured(args);
  EXPECT_DOUBLE_EQ(42.0, result.number());
}

TEST_F(InterpreterTest,
       ExceptionsThrownByCalledJsFunctionsAreReportedNotPropagated)
{
  run("capture(function() { throw new Error('callback boom'); });");
  ASSERT_TRUE(captured.isFunction());

  JSON::Array args;
  JSON::Value result;
  ASSERT_NO_THROW(result = captured(args));
  ASSERT_TRUE(result.isObject());
  ASSERT_TRUE(result.object().contains("exception"));
  EXPECT_NE(std::string::npos,
            result["exception"].string().find("callback boom"));
}

TEST_F(InterpreterTest, NativeClassesExposeConstructorMethodsAndProperties)
{
  auto& cls = addThingClass(*engine);
  cls.addMethod(
      "double", [](void* self, JSON::Array&) -> JSON::Value
      { return static_cast<double>(static_cast<Thing*>(self)->value * 2); });
  cls.addGetSet(
      "value", [](void* self) -> JSON::Value
      { return static_cast<double>(static_cast<Thing*>(self)->value); },
      [](void* self, JSON::Value& v)
      { static_cast<Thing*>(self)->value = static_cast<int>(v.number()); });

  JSON::Value v = run("var t = new Thing(21); var d = t.double(); t.value = 5; "
                      "[d, t.value].join(',');");
  EXPECT_EQ("42,5", v.string());
}

TEST_F(InterpreterTest, NativeInstancesReturnedFromApiFunctionsAreWrappedByType)
{
  auto& cls = addThingClass(*engine);
  cls.addGetSet(
      "value", [](void* self) -> JSON::Value
      { return static_cast<double>(static_cast<Thing*>(self)->value); },
      nullptr);

  auto shared = std::make_shared<Thing>();
  shared->value = 99;
  engine->addAPIFunction("getThing", [shared](JSON::Array&) -> JSON::Value
                         { return JSON::makeNative(shared); });

  EXPECT_DOUBLE_EQ(99.0, run("getThing().value;").number());
  // The same native pointer maps to the same JS wrapper.
  EXPECT_TRUE(run("getThing() === getThing();").boolean());

  // A native object handed back to C++ resolves to the original pointer
  // (tagged with the class's *base* type - void here, as for the app's
  // addClass<void, T>() extensions - so go through native() rather than
  // asNative<Thing>()).
  run("capture(getThing());");
  ASSERT_TRUE(captured.isNative());
  EXPECT_EQ(shared.get(), captured.native().first.get());
  EXPECT_EQ(std::type_index{typeid(void)}, captured.native().second);
}

TEST_F(InterpreterTest, NativeExceptionsBecomeCatchableJsErrors)
{
  engine->addAPIFunction("explode", [](JSON::Array&) -> JSON::Value
                         { throw std::runtime_error{"native boom"}; });

  JSON::Value v = run("var msg = 'none'; try { explode(); } catch (e) { msg = "
                      "e.message; } msg;");
  EXPECT_NE(std::string::npos, v.string().find("native boom"));
}

TEST_F(InterpreterTest, MethodsCalledOnTheWrongThisThrowInsteadOfCrashing)
{
  auto& cls = addThingClass(*engine);
  cls.addMethod("poke", [](void*, JSON::Array&) -> JSON::Value { return 1.0; });

  JSON::Value v = run("var ok = false; try { Thing.prototype.poke.call({}); } "
                      "catch (e) { ok = true; } ok;");
  EXPECT_TRUE(v.boolean());
}

TEST_F(InterpreterTest, NativeObjectsAreReleasedWhenTheInterpreterIsDestroyed)
{
  Thing::liveCount = 0;
  {
    auto local = di::inject<Interpreter>("js");
    addThingClass(*local);
    local->eval("var keep = [new Thing(), new Thing()];", "t",
                Interpreter::EvalType::Script);
    EXPECT_EQ(2, Thing::liveCount);
  }
  EXPECT_EQ(0, Thing::liveCount);
}

TEST_F(InterpreterTest, PromiseJobsRunBeforeEvalReturns)
{
  run("Promise.resolve(5).then(function(v) { capture(v * 2); });");
  EXPECT_DOUBLE_EQ(10.0, captured.number());
}

TEST_F(InterpreterTest, TickDrainsPendingJobs)
{
  // A job queued from native code (outside eval) only runs on the next
  // tick().
  run("capture(function() { Promise.resolve('ticked').then(function(v) { "
      "capture(v); }); });");
  ASSERT_TRUE(captured.isFunction());
  JSON::Value fn = captured;
  JSON::Array noArgs;
  fn(noArgs);

  engine->tick();
  ASSERT_TRUE(captured.isString());
  EXPECT_EQ("ticked", captured.string());
}

TEST_F(InterpreterTest, UnhandledRejectionsDoNotAbortTheProcess)
{
  // js_std's own tracker would exit(1) here; delta buffers and reports them.
  EXPECT_NO_THROW(run("Promise.reject(new Error('nobody catches me'));"));
  EXPECT_NO_THROW(engine->tick());
  EXPECT_DOUBLE_EQ(1.0, run("1;").number());
}

TEST_F(InterpreterTest, AddGlobalValueExposesPlainValues)
{
  JSON::Value cfg;
  cfg["name"] = "besprited";
  cfg["flag"] = true;
  engine->addGlobalValue("cfg", cfg);
  EXPECT_EQ("besprited:true", run("cfg.name + ':' + cfg.flag;").string());
}

int main(int argc, char** argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
