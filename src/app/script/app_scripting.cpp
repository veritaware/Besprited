// Aseprite    | Copyright (C) 2001-2016 David Capello
// LibreSprite | Copyright (C) 2021-2026 LibreSprite contributors
// Besprited   | Copyright (C) 2026      Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "delta/Extension.hpp"
#include "delta/Interpreter.hpp"
#include "delta/Shared.hpp"
#include "app/app.h"
#include "app/resource_finder.h"
#include "app/script/app_scripting.h"
#include "app/script/script_permission_dialog.h"
#include "app/script/script_permissions.h"
#include "app/task_manager.h"
#include "base/injection.h"
#include "base/path.h"
#include "base/range.h"
#include "base/string.h"
#include "base/trim_string.h"
#include "di.hpp"
#include "script/engine_delegate.h"
#include "ui/manager.h"
#include "ui/message.h"
#include "ui/message_type.h"
#include "ui/timer.h"
#include "ui/widget.h"

#include <chrono>
#include <cmath>
#include <deque>
#include <filesystem>
#include <fstream>
#include <memory>
#include <map>
#include <sstream>
#include <string>
#include <optional>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>

namespace
{

std::unordered_set<std::string> loadedScripts;
std::shared_ptr<Interpreter> engine;
std::vector<std::shared_ptr<Extension>> extensions;
JSON::Value LS;

// The file of the most recently loaded script ("" if none).
std::string activeScript;

// REPL result echo (set by printLastResult()).
bool printLastResultFlag = false;

// The registered event handler (setEventHandler). Undefined until called.
JSON::Value eventHandler;
// Script that registered eventHandler: its events run under its grants.
std::string eventHandlerOwner;
int evalFileDepth = 0;

struct PendingEvent
{
  std::string fileName;
  JSON::Value args;
};
Shared<std::deque<PendingEvent>> pendingEvents;

// Callbacks flushed after eval() returns (palette version bumps, deferred
// dialog builds, ...).
std::deque<std::function<void()>> afterEvalQueue;

class ScriptTimer : public ui::Timer
{
public:
  ScriptTimer()
    : ui::Timer(1)
  {
  }
  void onTick() override { app::AppScripting::tick(); }
};
std::unique_ptr<ScriptTimer> tickTimer;

// Canonical form of a script path (absolute + lexically normalized, the
// same form as AppScripting::activeScript). Used to match event file
// names against loadedScripts no matter how the path was passed in.
std::string canonicalScriptPath(const std::string& path)
{
  std::filesystem::path p{path};
  if (!p.is_absolute())
    p = std::filesystem::current_path() / p;
  return p.lexically_normal().string();
}

// Routes text to the console delegate (DevConsole panel, --shell stdout)
// with a plain stdout fallback when no delegate is registered.
void routeToConsole(const std::string& text)
{
  if (script::EngineDelegate::getRegistry().count(""))
  {
    inject<script::EngineDelegate> delegate;
    if (delegate)
    {
      delegate->onConsolePrint(text.c_str());
      return;
    }
  }
  std::cout << text << '\n';
}

std::string formatReplValue(JSON::Value& value)
{
  if (value.isString())
    return value.string();
  if (value.isNumber())
  {
    const double d = value.number();
    if (std::floor(d) == d && std::fabs(d) < 9007199254740992.0)
      return std::to_string(static_cast<long long>(d));
    std::ostringstream os;
    os << d;
    return os.str();
  }
  return value.toString();
}

} // anonymous namespace

namespace app
{
std::multimap<std::string, std::string> eventHooks;

class EventListener : public ui::Widget
{
public:
  bool onProcessMessage(ui::Message* msg) override
  {
    auto event = ui::to_string(msg->type());
    auto range = base::range{eventHooks.equal_range(event)};
    if (range.empty())
      return false;

    JSON::Value args;
    args.push_back(event);
    args.push_back(static_cast<double>(msg->modifiers()));

    switch (msg->type())
    {
    case ui::kKeyDownMessage:
    case ui::kKeyUpMessage:
      args.push_back(static_cast<double>(static_cast<ui::KeyMessage*>(msg)->unicodeChar()));
      args.push_back(static_cast<double>(static_cast<ui::KeyMessage*>(msg)->scancode()));
      break;

    case ui::kMouseDownMessage:
    case ui::kMouseUpMessage:
    case ui::kDoubleClickMessage:
    case ui::kMouseEnterMessage:
    case ui::kMouseLeaveMessage:
    case ui::kMouseMoveMessage:
    case ui::kSetCursorMessage:
    case ui::kMouseWheelMessage:
      args.push_back(static_cast<double>(static_cast<ui::MouseMessage*>(msg)->position().x));
      args.push_back(static_cast<double>(static_cast<ui::MouseMessage*>(msg)->position().y));
      args.push_back(static_cast<double>(static_cast<ui::MouseMessage*>(msg)->buttons()));
      args.push_back(static_cast<double>(static_cast<ui::MouseMessage*>(msg)->wheelDelta().x));
      args.push_back(static_cast<double>(static_cast<ui::MouseMessage*>(msg)->wheelDelta().y));
      break;

    default:
      break;
    }

    for (auto& entry : range)
      AppScripting::raiseEvent(entry.second, args);

    return false;
  }
};

std::unique_ptr<EventListener> listener;

void AppScripting::clearEventHooks()
{
  eventHooks.clear();
  if (listener)
  {
    ui::Manager::getDefault()->removeMessageListener(listener.get());
    listener.reset();
  }
}

void AppScripting::shutdown()
{
  // Stop the tick pump + drop the message listener before the ui::Manager
  // is torn down.
  tickTimer.reset();
  clearEventHooks();
  // Release JS references held in app-side state before the interpreter
  // (and its runtime) goes away.
  eventHandler = JSON::Value{JSON::Special::Undefined};
  pendingEvents << [](auto& q) { q.clear(); };
  for (auto& ext : extensions)
    ext->shutdown();
  extensions.clear();
  // Destroy the engine (and any script-owned dialogs/widgets it holds) so
  // they don't outlive the ui::Manager.
  engine.reset();
}

bool AppScripting::scanScript(const std::string& fullPath)
{
  auto extension = base::string_to_lower(base::get_file_extension(fullPath));
  if (extension != "js")
    return false;

  std::ifstream file{fullPath};

  if (!file)
    return false;

  std::string line;
  while (std::getline(file, line))
  {
    base::trim_string(line, line);
    auto parts = base::split(line, ' ');

    for (auto& part : parts)
      base::trim_string(part, part);

    if (parts.empty() || parts[0].size() < 3 || !parts[0].starts_with("///"))
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

    if (key == "listen")
    {
      if (!listener)
      {
        listener = std::make_unique<EventListener>();
        ui::Manager::getDefault()->addMessageListener(listener.get());
      }
      for (auto& event : parts)
      {
        eventHooks.insert({event, fullPath});
      }
    }
  }

  return true;
}

void AppScripting::initEngine()
{
  if (engine)
    return;

  App::instance()->Exit.connect(
      []
      {
        // Drop the timer while the UI Manager is still alive.
        tickTimer.reset();
        // Release JS references held in app-side state before the
        // interpreter (and its runtime) goes away.
        eventHandler = JSON::Value{JSON::Special::Undefined};
        pendingEvents << [](auto& q) { q.clear(); };
        for (auto& ext : extensions)
          ext->shutdown();
        extensions.clear();
        engine.reset();
      });

  engine = di::inject<Interpreter>("js");

  // Module search paths for bare import specifiers: the user's scripts
  // dir first, then the bundled data/scripts (mirrors ResourceFinder's
  // user/data dir resolution).
  {
    ResourceFinder rf{false};
    rf.includeUserDir("scripts");
    rf.includeDataDir("scripts");
    while (rf.next())
      engine->addModuleSearchPath(rf.filename());
  }

  LS["version"] = VERSION;
  LS["package"] = PACKAGE;
  engine->addGlobalValue("LS", LS);

  // Core event API: a single handler for all engine events.
  // setEventHandler(fn, ifEmpty?): with ifEmpty, only registers fn when
  // no handler is set yet (used by the onEvent auto-inject epilogue so
  // an explicit setEventHandler in the script wins).
  engine->addAPIFunction("setEventHandler",
                         [](JSON::Array& args) -> JSON::Value
                         {
                           if (args.empty() || !args[0].isFunction())
                             return JSON::Value{JSON::Special::Undefined};
                           const bool ifEmpty = args.size() > 1 && args[1].boolean();
                           if (ifEmpty && !eventHandler.isUndefined())
                             return JSON::Value{JSON::Special::Undefined};
                           eventHandler = args[0];
                           eventHandlerOwner = activeScript;
                           return JSON::Value{JSON::Special::Undefined};
                         });

  extensions = di::injectAll<Extension>();
  for (auto& ext : extensions)
    addExtension(*ext);
}

void AppScripting::addExtension(Extension& ext)
{
  for (auto& [name, func] : ext.getFunctions())
    engine->addAPIFunction(name, func);
  for (auto& [className, clazz] : ext.getClasses())
  {
    auto& cls = engine->addClass(className, clazz->getBaseType(), clazz->getDerivedType(), clazz->getConstructor());
    for (auto& [name, method] : clazz->getMethods())
    {
      cls.addMethod(name, method);
    }
    for (auto& [name, getset] : clazz->getGetSet())
    {
      cls.addGetSet(name, getset.get, getset.set);
    }
  }
  auto boot = ext.init("js", LS);
  if (!boot.empty())
  {
    try
    {
      engine->eval(boot, typeid(ext).name());
    }
    catch (const std::exception& e)
    {
      std::cerr << "Warning: Failed to run boot script for extension " << typeid(ext).name() << ": " << e.what()
                << '\n';
    }
  }
}

void AppScripting::drainEventQueue()
{
  for (;;)
  {
    PendingEvent ev;
    bool got = false;
    pendingEvents << [&](auto& q)
    {
      if (!q.empty())
      {
        ev = std::move(q.front());
        q.pop_front();
        got = true;
      }
    };
    if (!got)
      break;

    // Lazily load the script that owns this event (first-event load for
    // ///listen scripts). Re-queue the event so the "init" event fired
    // by evalFile is delivered before it (legacy load-then-fire order).
    // Normalize the path the same way evalFile stores it so a
    // relative/absolute mismatch does not reload an already loaded script.
    auto normPath = ev.fileName.empty() ? std::string{} : canonicalScriptPath(ev.fileName);
    if (!normPath.empty() && loadedScripts.find(normPath) == loadedScripts.end())
    {
      if (evalFile(normPath))
      {
        pendingEvents << [&](auto& q) { q.push_back(std::move(ev)); };
        continue;
      }
      continue; // file could not be loaded; drop the event
    }

    if (!eventHandler.isUndefined())
    {
      std::optional<ScriptPermissions::ScopedContext> permContext;
      if (!eventHandlerOwner.empty())
        permContext.emplace(eventHandlerOwner);
      JSON::Value ret = eventHandler(ev.args.array());
      // The delta JS->C++ wrapper converts a thrown JS exception into
      // an object with an "exception" key; surface it on the console.
      if (ret.isObject() && ret.object().contains("exception"))
        routeToConsole(ret["exception"].string());
    }
  }
}

void AppScripting::flushAfterEval()
{
  while (!afterEvalQueue.empty())
  {
    auto fn = std::move(afterEvalQueue.front());
    afterEvalQueue.pop_front();
    fn();
  }
}

void AppScripting::raiseEvent(const std::string& fileName, script::Value& event)
{
  initEngine();
  if (!engine)
    return;
  pendingEvents << [&](auto& q) { q.emplace_back(fileName, event); };
}

bool AppScripting::eval(const std::string& code, const std::string& path)
{
  // REPL lines are user-typed code: not gated by script permissions,
  // but only for the duration of the line.
  ScriptPermissions::ScopedContext permContext;
  ScriptPermissions::setUnrestricted();
  initEngine();
  if (!engine)
    return false;
  loadedScripts.insert(path);
  try
  {
    // REPL lines eval as scripts: top-level `var` persists across lines
    // (matching the legacy REPL), and the last expression's completion
    // value is returned.
    JSON::Value result = engine->eval(code, path, Interpreter::EvalType::Script);
    if (printLastResultFlag && !result.isUndefined())
      routeToConsole(formatReplValue(result));
    flushAfterEval();
    return true;
  }
  catch (const std::exception& e)
  {
    routeToConsole(e.what());
    return false;
  }
}

bool AppScripting::evalFile(const std::string& fileName)
{
  initEngine();
  if (!engine)
    return false;
  std::cout << "Reading file " << fileName << '\n';
  std::ifstream ifs(fileName);
  if (!ifs)
  {
    std::cout << "Could not open " << fileName << '\n';
    return false;
  }
  auto source = std::string{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};

  // Backward-compat epilogue: a legacy `function onEvent` becomes the
  // event handler unless the script already called setEventHandler.
  // Appended (not prepended) so `let`/`const` declarations in the script
  // are not shadowed, and `typeof onEvent` is safe at this point.
  source += "\nif (typeof onEvent === \"function\") setEventHandler(onEvent, "
            "true);\n";

  // Absolutize the entry path: it is the base for the entry module's
  // relative imports.
  std::string absPath = fileName;
  if (!std::filesystem::path(fileName).is_absolute())
    absPath = (std::filesystem::current_path() / fileName).string();

  // Lexically normalized: it becomes the default storage domain, whose
  // path-traversal check rejects ".." (see issue #219).
  auto canonicalPath = std::filesystem::path(absPath).lexically_normal().string();

  // Permission gate: the script requests API access via "///permission"
  // header lines; the first run prompts for the missing grants (or
  // auto-grants them when there is no UI to prompt in). A cancelled
  // prompt aborts the run.
  auto requested = ScriptPermissions::parseRequested(canonicalPath);
  if (!ensurePermissions(canonicalPath, requested))
    return false;

  // A nested run (e.g. via RunScript) must not leave its grants or its
  // active script behind for the caller.
  std::optional<ScriptPermissions::ScopedContext> callerContext;
  const std::string callerScript = activeScript;
  const bool nested = evalFileDepth > 0;
  if (nested)
    callerContext.emplace();
  ScriptPermissions::setActiveContext(canonicalPath, ScriptPermissions::granted(canonicalPath));
  ++evalFileDepth;
  struct DepthGuard
  {
    ~DepthGuard() { --evalFileDepth; }
  } depthGuard;

  // Re-run semantics: the most recently run script owns the event
  // handler, so drop the previous one and let the epilogue / an
  // explicit setEventHandler in the script re-register it. (The
  // event-driven lazy load in drainEventQueue only calls evalFile for
  // files not in loadedScripts, so this never re-evaluates on events.)
  eventHandler = JSON::Value{JSON::Special::Undefined};

  activeScript = canonicalPath;

  bool ok = false;
  try
  {
    engine->eval(source, absPath, Interpreter::EvalType::Module);
    ok = true;
  }
  catch (const std::exception& e)
  {
    routeToConsole(e.what());
  }

  if (ok)
  {
    // Store the canonical path (the same form events carry via
    // getFileName), so event-driven lookups do not re-load the script.
    loadedScripts.insert(activeScript);
    JSON::Value initEvent;
    initEvent.push_back("init");
    pendingEvents << [&](auto& q) { q.emplace_back(fileName, initEvent); };
    // Deliver "init" immediately (legacy semantics fired it
    // synchronously during evalFile).
    drainEventQueue();
    flushAfterEval();
  }
  if (nested)
    activeScript = callerScript;
  return ok;
}

bool AppScripting::ensurePermissions(const std::string& path, const std::set<std::string>& requested)
{
  auto granted = ScriptPermissions::granted(path);
  std::set<std::string> missing;
  for (auto& perm : requested)
    if (!granted.count(perm))
      missing.insert(perm);
  if (missing.empty())
    return true;

  if (!ui::Manager::getDefault())
  {
    // Headless (--script without a UI): auto-grant so batch runs keep
    // working; the prompt is a GUI-only concern. The grant is not saved,
    // so a later GUI run of the same script still asks.
    for (auto& perm : missing)
      granted.insert(perm);
    ScriptPermissions::setSessionGranted(path, granted);
    std::string list;
    for (auto& perm : missing)
      list += (list.empty() ? "" : " ") + perm;
    std::cout << "Warning: no UI to prompt for permissions; auto-granted " << list << " for " << path << std::endl;
    return true;
  }

  std::set<std::string> chosen;
  if (!ScriptPermissionDialog::show(path, missing, chosen))
    return false;
  for (auto& perm : chosen)
    granted.insert(perm);
  ScriptPermissions::setGranted(path, granted);
  return true;
}

void AppScripting::printLastResult()
{
  printLastResultFlag = true;
}

void AppScripting::tick()
{
  if (!engine)
    return;
  engine->tick();
  drainEventQueue();
  for (auto& ext : extensions)
    ext->tick();
}

void AppScripting::startTickPump()
{
  if (tickTimer)
    return;
  if (!ui::Manager::getDefault())
    return;
  tickTimer = std::make_unique<ScriptTimer>();
  tickTimer->start();
}

void AppScripting::runHeadlessPump()
{
  if (!engine)
    return;
  using namespace std::chrono;
  for (;;)
  {
    // Deliver worker-thread results (fetch, fs, server handlers) queued
    // by TaskManager; in GUI mode its 10ms ui::Timer does this.
    if (app::TaskManager::exists())
      app::TaskManager::instance().pump();

    // Same sequence as tick() (which the GUI runs per frame): pump the
    // engine (promises, microtasks), deliver queued events, then ask each
    // extension for its next wake time (0 = idle) while firing due work.
    engine->tick();
    drainEventQueue();
    milliseconds nextWake{0};
    for (auto& ext : extensions)
    {
      auto wake = ext->tick();
      if (wake > milliseconds(0) && (nextWake == milliseconds(0) || wake < nextWake))
        nextWake = wake;
    }
    // Settle the work queued by the above (e.g. a fired timer resolving a
    // promise) before checking for idle.
    engine->tick();

    // The script is active while work is pending: events to deliver,
    // due extension work, tasks in TaskManager (fs, fetch, server
    // handlers), or a keepAlive() extension (e.g. a listening HTTP
    // server).
    bool active = nextWake > milliseconds(0);
    pendingEvents >> [&](auto& q)
    {
      if (!q.empty())
        active = true;
    };
    if (app::TaskManager::exists() && app::TaskManager::instance().hasWork())
      active = true;
    if (!active)
    {
      for (auto& ext : extensions)
      {
        if (ext->keepAlive())
        {
          active = true;
          break;
        }
      }
    }
    if (!active)
      break;

    // Sleep until the next wake, capped at the TaskManager poll period so
    // worker results are delivered with the same latency as in GUI mode.
    milliseconds sleepFor = nextWake > milliseconds(0) ? nextWake : milliseconds(10);
    if (sleepFor > milliseconds(10))
      sleepFor = milliseconds(10);
    std::this_thread::sleep_for(sleepFor);
  }
}

std::string AppScripting::getFileName()
{
  return activeScript;
}

void AppScripting::afterEval(std::function<void()> fn)
{
  afterEvalQueue.emplace_back(std::move(fn));
}

void AppScripting::consolePrint(const std::string& text)
{
  routeToConsole(text);
}

} // namespace app
