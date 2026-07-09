// LibreSprite Scripting Library
// Copyright (C) 2021-2026  LibreSprite contributors
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <iostream>
#include <string>
#include <unordered_map>

#include <quickjs.h>

#include "base/convert_to.h"
#include "base/exception.h"
#include "base/memory.h"
#include "script/engine.h"
#include "script/engine_delegate.h"

using namespace script;

namespace {
  JSClassID g_scriptObjectClassId;
  JSClassID g_functionClassId;
  bool g_classIdsAllocated = false;
}

static JSValue returnValue(JSContext* ctx, const Value& value);
static Value getValue(JSContext* ctx, JSValueConst v);

class QuickJSScriptObject;

class QuickJSEngine : public Engine {
public:
  inject<EngineDelegate> m_delegate;
  JSRuntime* m_runtime;
  JSContext* m_context;

  QuickJSEngine() {
    InternalScriptObject::setDefault("QuickJSScriptObject");
    m_runtime = JS_NewRuntime();
    m_context = JS_NewContext(m_runtime);
    registerClasses();
  }

  ~QuickJSEngine() {
    JS_FreeContext(m_context);
    JS_FreeRuntime(m_runtime);
  }

  void registerClasses() {
    if (!g_classIdsAllocated) {
      JS_NewClassID(m_runtime, &g_scriptObjectClassId);
      JS_NewClassID(m_runtime, &g_functionClassId);
      g_classIdsAllocated = true;
    }

    JSClassDef objectClassDef = {};
    objectClassDef.class_name = "LibreSpriteObject";
    objectClassDef.finalizer = &QuickJSEngine::onObjectFinalized;
    JS_NewClass(m_runtime, g_scriptObjectClassId, &objectClassDef);

    // Holds a raw script::Function* used as closure data for native
    // callbacks. The Function's lifetime is owned by InternalScriptObject,
    // so this class needs no finalizer of its own.
    JSClassDef funcClassDef = {};
    funcClassDef.class_name = "LibreSpriteFunctionPtr";
    JS_NewClass(m_runtime, g_functionClassId, &funcClassDef);
  }

  static void onObjectFinalized(JSRuntime* rt, JSValue val);

  void reportException() {
    JSValue exc = JS_GetException(m_context);
    std::string message;

    const char* str = JS_ToCString(m_context, exc);
    if (str) {
      message = str;
      JS_FreeCString(m_context, str);
    } else {
      message = "Unknown error";
    }

    JSValue stack = JS_GetPropertyStr(m_context, exc, "stack");
    if (!JS_IsUndefined(stack)) {
      const char* stackStr = JS_ToCString(m_context, stack);
      if (stackStr) {
        message += "\n";
        message += stackStr;
        JS_FreeCString(m_context, stackStr);
      }
    }
    JS_FreeValue(m_context, stack);
    JS_FreeValue(m_context, exc);

    m_delegate->onConsolePrint(message.c_str());
    std::cout << "Error: [" << message << "]" << std::endl;
  }

  bool raiseEvent(const std::vector<script::Value>& event) override {
    bool success = true;
    JSValue global = JS_GetGlobalObject(m_context);
    JSValue onEvent = JS_GetPropertyStr(m_context, global, "onEvent");

    if (JS_IsFunction(m_context, onEvent)) {
      std::vector<JSValue> argv;
      argv.reserve(event.size());
      for (auto& arg : event)
        argv.push_back(returnValue(m_context, arg));

      JSValue result = JS_Call(m_context, onEvent, global, static_cast<int>(argv.size()), argv.data());
      for (auto& arg : argv)
        JS_FreeValue(m_context, arg);

      if (JS_IsException(result)) {
        reportException();
        success = false;
      }
      JS_FreeValue(m_context, result);
    }

    JS_FreeValue(m_context, onEvent);
    JS_FreeValue(m_context, global);
    execAfterEval(success);
    return success;
  }

  bool eval(const std::string& code) override {
    bool success = true;
    initGlobals();

    JSValue result = JS_Eval(m_context, code.c_str(), code.size(), "input.js", JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(result)) {
      reportException();
      success = false;
    } else if (m_printLastResult && !JS_IsUndefined(result)) {
      const char* str = JS_ToCString(m_context, result);
      if (str) {
        m_delegate->onConsolePrint(str);
        JS_FreeCString(m_context, str);
      }
    }
    JS_FreeValue(m_context, result);

    execAfterEval(success);
    return success;
  }
};

static Engine::Regular<QuickJSEngine> registration("qjs", {"js"});

class QuickJSScriptObject : public InternalScriptObject {
  int m_refC = 0;

public:
  static JSValue callFunc(JSContext* ctx, JSValueConst /*thisVal*/, int argc, JSValueConst* argv,
                           int /*magic*/, JSValue* funcData) {
    auto func = reinterpret_cast<script::Function*>(JS_GetOpaque(funcData[0], g_functionClassId));

    for (int i = 0; i < argc; ++i)
      func->arguments.push_back(getValue(ctx, argv[i]));

    func->result.makeUndefined();
    try {
      (*func)();
    } catch (const ObjectDestroyedException&) {
      std::cout << "Object Destroyed Exception" << std::endl;
    }

    return returnValue(ctx, func->result);
  }

  JSValue makeFuncValue(JSContext* ctx, script::Function& func) {
    JSValue holder = JS_NewObjectClass(ctx, g_functionClassId);
    JS_SetOpaque(holder, &func);
    // JS_NewCFunctionData duplicates the values it needs from `holder`
    // internally, so our local reference must still be released.
    JSValue jsFunc = JS_NewCFunctionData(ctx, &QuickJSScriptObject::callFunc, 0, 0, 1, &holder);
    JS_FreeValue(ctx, holder);
    return jsFunc;
  }

  void pushFunctions(JSContext* ctx, JSValueConst object) {
    for (auto& entry : functions) {
      JSValue func = makeFuncValue(ctx, entry.second);
      JS_SetPropertyStr(ctx, object, entry.first.c_str(), func);
    }
  }

  void pushProperties(JSContext* ctx, JSValueConst object) {
    for (auto& entry : properties) {
      JSAtom atom = JS_NewAtom(ctx, entry.first.c_str());
      JSValue getter = makeFuncValue(ctx, entry.second.getter);
      JSValue setter = makeFuncValue(ctx, entry.second.setter);
      JS_DefinePropertyGetSet(ctx, object, atom, getter, setter, JS_PROP_CONFIGURABLE);
      JS_FreeAtom(ctx, atom);
    }
  }

  JSValue makeLocal(JSContext* ctx) {
    JSValue object = JS_NewObjectClass(ctx, g_scriptObjectClassId);
    JS_SetOpaque(object, this);
    ++m_refC;
    pushFunctions(ctx, object);
    pushProperties(ctx, object);
    return object;
  }

  void release() {
    if (!--m_refC && onRelease) {
      auto cb = std::move(onRelease);
      onRelease = nullptr;
      cb();
    }
  }

  void makeGlobal(const std::string& name) override {
    auto engine = m_engine.get<QuickJSEngine>();
    JSValue global = JS_GetGlobalObject(engine->m_context);
    JSValue object = makeLocal(engine->m_context);
    JS_SetPropertyStr(engine->m_context, global, name.c_str(), object);
    JS_FreeValue(engine->m_context, global);
  }
};

void QuickJSEngine::onObjectFinalized(JSRuntime* rt, JSValue val) {
  auto self = reinterpret_cast<QuickJSScriptObject*>(JS_GetOpaque(val, g_scriptObjectClassId));
  if (self)
    self->release();
}

static Value getValue(JSContext* ctx, JSValueConst v) {
  if (JS_IsNull(v) || JS_IsUndefined(v))
    return {};

  if (JS_IsString(v)) {
    const char* str = JS_ToCString(ctx, v);
    Value result = str ? Value{str} : Value{};
    JS_FreeCString(ctx, str);
    return result;
  }

  if (JS_IsBool(v))
    return JS_ToBool(ctx, v) ? 1 : 0;

  if (JS_IsNumber(v)) {
    double d = 0;
    JS_ToFloat64(ctx, &d, v);
    return d;
  }

  if (JS_IsObject(v)) {
    // Probe for a typed array by asking for its backing ArrayBuffer. This
    // throws (and sets a pending exception) when `v` isn't a typed array,
    // so the probe exception is discarded before falling back to generic
    // object enumeration below.
    size_t byteOffset = 0, byteLength = 0, bytesPerElement = 0;
    JSValue arrayBuffer = JS_GetTypedArrayBuffer(ctx, v, &byteOffset, &byteLength, &bytesPerElement);
    if (!JS_IsException(arrayBuffer)) {
      size_t size = 0;
      uint8_t* data = JS_GetArrayBuffer(ctx, &size, arrayBuffer);
      Value result;
      if (data)
        result = Value{data + byteOffset, byteLength, false};
      JS_FreeValue(ctx, arrayBuffer);
      return result;
    }
    JS_FreeValue(ctx, JS_GetException(ctx));

    JSPropertyEnum* tab = nullptr;
    uint32_t count = 0;
    if (JS_GetOwnPropertyNames(ctx, &tab, &count, v, JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) == 0) {
      auto objptr = new std::unordered_map<std::string, Value>[1];
      auto& obj = objptr[0];
      for (uint32_t i = 0; i < count; ++i) {
        JSValue val = JS_GetProperty(ctx, v, tab[i].atom);
        const char* key = JS_AtomToCString(ctx, tab[i].atom);
        if (key)
          obj[key] = getValue(ctx, val);
        JS_FreeCString(ctx, key);
        JS_FreeValue(ctx, val);
        JS_FreeAtom(ctx, tab[i].atom);
      }
      js_free(ctx, tab);
      return {objptr, true};
    }
  }

  return {};
}

static JSValue returnValue(JSContext* ctx, const Value& value) {
  switch (value.type) {

  case Value::Type::UNDEFINED:
    return JS_UNDEFINED;

  case Value::Type::INT:
    return JS_NewInt32(ctx, value);

  case Value::Type::DOUBLE:
    return JS_NewFloat64(ctx, value);

  case Value::Type::STRING: {
    const char* str = value;
    return JS_NewStringLen(ctx, str, value.size());
  }

  case Value::Type::OBJECT:
    if (auto object = static_cast<ScriptObject*>(value)) {
      return static_cast<QuickJSScriptObject*>(object->getInternalScriptObject())->makeLocal(ctx);
    }
    return JS_NULL;

  case Value::Type::BUFFER: {
    auto& buffer = value.buffer();
    JSValue arrayBuffer;
    if (buffer.canSteal()) {
      arrayBuffer = JS_NewArrayBuffer(ctx, buffer.steal(), buffer.size(),
        [](JSRuntime*, void* /*opaque*/, void* ptr) {
          delete[] static_cast<uint8_t*>(ptr);
        }, nullptr, false);
    } else {
      arrayBuffer = JS_NewArrayBufferCopy(ctx, buffer.data(), buffer.size());
    }

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue ctor = JS_GetPropertyStr(ctx, global, "Uint8Array");
    JSValue args[1] = { arrayBuffer };
    JSValue typedArray = JS_CallConstructor(ctx, ctor, 1, args);
    JS_FreeValue(ctx, ctor);
    JS_FreeValue(ctx, global);
    JS_FreeValue(ctx, arrayBuffer);
    return typedArray;
  }

  default:
    printf("Unknown return type: %d\n", int(value.type));
    break;
  }
  return JS_UNDEFINED;
}

static InternalScriptObject::Regular<QuickJSScriptObject> qjsSO("QuickJSScriptObject");
