// Copyright (C) 2026 Veritaware
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License version 2 as
// published by the Free Software Foundation.

#define TEST_GUI
#include "tests.h"

#include "app/app.h"
#include "app/commands/commands.h"
#include "app/document.h"
#include "app/script/app_scripting.h"
#include "app/script/api/script_api_common.h"
#include "app/script/script_permissions.h"
#include "app/ui_context.h"
#include "base/launcher.h"
#include "delta/Extension.hpp"
#include "delta/JSON.hpp"
#include "delta/Interpreter.hpp"
#include "di.hpp"
#include "doc/cel.h"
#include "doc/color.h"
#include "doc/documents.h"
#include "doc/image.h"
#include "doc/layer.h"
#include "doc/primitives.h"
#include "doc/site.h"
#include "doc/sprite.h"
#include "she/system.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <cstring>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using app::AppScripting;

namespace
{

// Bridges the test's C++ side and JS. It is a regular script `Extension`,
// picked up by AppScripting::initEngine() through di::injectAll<Extension>()
// exactly like app's own api/*_script.cpp extensions:
//   - "native.capture(x)" records whatever JS passes so the test can assert
//     on it afterwards;
//   - "native.image" hands the Image wrapper for the slot set by the test
//     beforehand (see AppScriptApiTest::useImage) to JS the same way
//     app.activeImage does for the real active document.
struct TestBridge
{
  static inline JSON::Value captured;
  static inline doc::Site site;
};

class TestBridgeExtension : public Extension
{
public:
  TestBridgeExtension()
  {
    addFunction("__testCapture") = [](JSON::Value& v) -> JSON::Value
    {
      TestBridge::captured = v;
      return {};
    };
    addFunction("__testImage") = []() -> JSON::Value
    { return JSON::makeNative(script_api::ImageSite::wrap(TestBridge::site)); };
  }

  std::string init(const std::string& language, JSON::Value&) override
  {
    if (language != "js")
      return "";
    return R"(
      globalThis.native = {
        capture: __testCapture,
        get image() { return __testImage(); }
      };
    )";
  }
};

di::provide<Extension, TestBridgeExtension> testBridgeExt{"testbridge"};

// AppScripting::initEngine() connects to App::instance()->Exit, and the
// "command" extension's constructor iterates every registered Command via
// app::CommandsModule::instance(), which is otherwise null. Like
// fill_stroke_mask_tests.cpp's App, both are deliberately leaked and
// constructed at most once for the whole binary (CommandsModule's own
// ASSERT(m_instance == NULL) means it can't safely be built more than once
// per process either).
void ensureAppAndCommands()
{
  static app::App* app = new app::App();
  app->initializeCoreModulesForTesting();
  static app::CommandsModule* commands = new app::CommandsModule();
  (void)commands;
  // The Image wrapper's writes are undoable and need the UI context.
  static app::UIContext* uiContext = new app::UIContext();
  (void)uiContext;
}

class AppScriptApiTest : public ::testing::Test
{
protected:
  static void SetUpTestSuite() { ensureAppAndCommands(); }

  void SetUp() override
  {
    TestBridge::captured = JSON::Value{JSON::Special::Undefined};
    TestBridge::site = doc::Site();
  }

  void TearDown() override
  {
    TestBridge::site = doc::Site();
    for (auto* d : docs)
      closeDoc(d);
    docs.clear();
  }

  // Creates a w x h RGB document filled with `color`, points
  // "native.image" at its first layer's cel at frame 0 and returns that
  // image. The wrapper only holds ids, so the document is owned here.
  doc::Image* useImage(int w, int h, doc::color_t color)
  {
    auto* d = static_cast<app::Document*>(app::UIContext::instance()->documents().add(w, h));
    docs.push_back(d);
    auto* layer = d->sprite()->folder()->getFirstLayer();
    TestBridge::site.document(d);
    TestBridge::site.sprite(d->sprite());
    TestBridge::site.layer(layer);
    TestBridge::site.frame(doc::frame_t(0));
    doc::Image* img = TestBridge::site.image();
    doc::clear_image(img, color);
    return img;
  }

  // Frees a document the way closing it would; handles into it must throw.
  void closeDoc(app::Document* d)
  {
    if (!d)
      return;
    app::UIContext::instance()->documents().remove(d);
    delete d;
    std::erase(docs, d);
  }

  std::vector<app::Document*> docs;

  // The engine is process-wide (one interpreter behind AppScripting); each
  // eval is a REPL-style script eval, so `var`s persist across calls.
  bool eval(const std::string& code) { return AppScripting::eval(code, "script_api_tests"); }

  JSON::Value& captured() { return TestBridge::captured; }
};

} // namespace

TEST_F(AppScriptApiTest, EvalReportsSuccessAndFailure)
{
  EXPECT_TRUE(eval("1 + 1;"));
  EXPECT_FALSE(eval("this is not valid javascript {{{"));
  EXPECT_FALSE(eval("throw new Error('boom');"));
  EXPECT_TRUE(eval("2;")) << "engine must keep working after a failed eval";
}

TEST_F(AppScriptApiTest, BridgeCapturesValuesFromJs)
{
  ASSERT_TRUE(eval("native.capture('hello');"));
  EXPECT_EQ("hello", captured().string());

  ASSERT_TRUE(eval("native.capture(42);"));
  EXPECT_EQ(42, static_cast<int>(captured()));
}

TEST_F(AppScriptApiTest, PixelColorRgbaRoundTripsThroughAllChannelAccessors)
{
  ASSERT_TRUE(eval("native.capture(app.pixelColor.rgba(10, 20, 30, 200));"));
  EXPECT_EQ(doc::rgba(10, 20, 30, 200), static_cast<doc::color_t>(captured().number()));

  ASSERT_TRUE(eval("var c = app.pixelColor.rgba(10, 20, 30, 200);"
                   "native.capture([app.pixelColor.rgbaR(c), app.pixelColor.rgbaG(c),"
                   "                app.pixelColor.rgbaB(c), "
                   "app.pixelColor.rgbaA(c)].join(','));"));
  EXPECT_EQ("10,20,30,200", captured().string());
}

TEST_F(AppScriptApiTest, PixelColorGrayaRoundTrips)
{
  ASSERT_TRUE(eval("var g = app.pixelColor.graya(128, 64);"
                   "native.capture([app.pixelColor.grayaV(g), "
                   "app.pixelColor.grayaA(g)].join(','));"));
  EXPECT_EQ("128,64", captured().string());
}

TEST_F(AppScriptApiTest, ColorModeConstantsMatchTheDocPixelFormatEnum)
{
  ASSERT_TRUE(eval("native.capture([ColorMode.RGB, ColorMode.GRAYSCALE, "
                   "ColorMode.INDEXED, ColorMode.BITMAP].join(','));"));
  std::string expected = std::to_string(int(doc::IMAGE_RGB)) + "," + std::to_string(int(doc::IMAGE_GRAYSCALE)) + "," +
                         std::to_string(int(doc::IMAGE_INDEXED)) + "," + std::to_string(int(doc::IMAGE_BITMAP));
  EXPECT_EQ(expected, captured().string());
}

TEST_F(AppScriptApiTest, AppVersionAndPlatformAreNonEmptyStrings)
{
  ASSERT_TRUE(eval("native.capture(app.version);"));
  EXPECT_FALSE(captured().string().empty());

  ASSERT_TRUE(eval("native.capture(app.platform);"));
  EXPECT_FALSE(captured().string().empty());
}

TEST_F(AppScriptApiTest, LegacyLsGlobalCarriesVersionAndPackage)
{
  ASSERT_TRUE(eval("native.capture(LS.version === app.version && typeof "
                   "LS.package === 'string' && LS.package.length > 0);"));
  EXPECT_TRUE(captured().boolean());
}

TEST_F(AppScriptApiTest, CommandSetParameterAndClearParametersAreChainable)
{
  // No UIContext exists in this headless test, so actually running a
  // command would just no-op (the command methods return 0 when
  // UIContext::instance() is null) - this only pins that
  // setParameter()/clearParameters() return something that itself has
  // "clearParameters" (i.e. the same command object), so JS call chains
  // like app.command.setParameter(...).clearParameters() work without
  // throwing.
  ASSERT_TRUE(eval("native.capture(typeof app.command.setParameter('a', "
                   "'b').clearParameters);"));
  EXPECT_EQ("function", captured().string());

  ASSERT_TRUE(eval("native.capture(app.command.setParameter('a', 'b') === app.command);"));
  EXPECT_TRUE(captured().boolean());
}

TEST_F(AppScriptApiTest, ConsoleLogIsAvailable)
{
  ASSERT_TRUE(eval("native.capture(typeof console.log + ',' + typeof console.assert);"));
  EXPECT_EQ("function,function", captured().string());
  EXPECT_TRUE(eval("console.log('script_api_tests: console.log works');"));
}

TEST_F(AppScriptApiTest, AppExposesTheDialogAndCommandEntryPoints)
{
  // Only probe for presence: TEST_GUI's ui::Manager has no she::System /
  // display behind it, so actually building a dialog is off limits here.
  ASSERT_TRUE(eval("native.capture([typeof app.createDialog, typeof "
                   "app.command, typeof app.open].join(','));"));
  EXPECT_EQ("function,object,function", captured().string());
}

TEST_F(AppScriptApiTest, CloseFileCloseAllFilesAndExitAreNotScriptReachable)
{
  // These delete Document/Sprite/Layer/Image objects a script may still
  // hold a live (non-owning) wrapper for - see issue #219. They must not
  // be exposed through the generic per-command enumeration, unlike an
  // ordinary command such as NewFile.
  ASSERT_TRUE(eval("native.capture([typeof command.CloseFile, typeof "
                   "command.CloseAllFiles, typeof command.Exit, typeof "
                   "command.NewFile].join(','));"));
  EXPECT_EQ("undefined,undefined,undefined,function", captured().string());
}

TEST_F(AppScriptApiTest, AppLaunchRequiresPermissionForUrlsAndOtherTargets)
{
  // A script holding no grants must not reach base::launcher::open_file()
  // with any target - a script-only RCE primitive when combined with
  // storage.save() (see issue #219). Accepted targets aren't tested: that
  // would actually invoke the OS launcher, unsafe from a headless CI test.
  Interpreter::TokenGuard guard(app::ScriptPermissions::tokenFor("script_api_tests_no_grants"));
  ASSERT_TRUE(eval("native.capture([app.launch('javascript:1'), "
                   "app.launch('https://example.com'), "
                   "app.launch('/tmp/should-not-be-touched'), "
                   "app.launch('')].join(','));"));
  EXPECT_EQ("false,false,false,false", captured().string());
}

TEST_F(AppScriptApiTest, ImageGetPixelPutPixelRoundTrip)
{
  useImage(4, 4, 0);

  ASSERT_TRUE(eval("native.image.putPixel(1, 1, app.pixelColor.rgba(5, 6, 7, 255));"
                   "native.capture(native.image.getPixel(1, 1));"));
  EXPECT_EQ(doc::rgba(5, 6, 7, 255), static_cast<doc::color_t>(captured().number()));

  ASSERT_TRUE(eval("native.capture([native.image.width, native.image.height].join('x'));"));
  EXPECT_EQ("4x4", captured().string());
}

TEST_F(AppScriptApiTest, ImagePutImageDataRejectsAWronglySizedBufferWithoutCorruptingTheImage)
{
  auto* img = useImage(4, 4, doc::rgba(1, 2, 3, 4));

  // A 1-byte buffer can never match a 4x4 RGBA image's byte size - the
  // size-mismatch guard in ImageExtension's putImageData() must reject it
  // and leave every pixel exactly as it was.
  ASSERT_TRUE(eval("native.image.putImageData(new Uint8Array(1));"));

  img = TestBridge::site.image();
  EXPECT_EQ(doc::rgba(1, 2, 3, 4), img->getPixel(0, 0));
  EXPECT_EQ(doc::rgba(1, 2, 3, 4), img->getPixel(3, 3));
}

TEST_F(AppScriptApiTest, ImageGetImageDataThenPutImageDataRoundTripsEveryPixel)
{
  auto* img = useImage(2, 2, 0);
  img->putPixel(0, 0, doc::rgba(1, 2, 3, 255));
  img->putPixel(1, 0, doc::rgba(4, 5, 6, 255));
  img->putPixel(0, 1, doc::rgba(7, 8, 9, 255));
  img->putPixel(1, 1, doc::rgba(10, 11, 12, 255));

  // Correctly-sized data must be accepted: round trip through JS, then
  // clear the image natively and write the captured bytes straight back.
  ASSERT_TRUE(eval("native.capture(native.image.getImageData());"));
  ASSERT_TRUE(captured().isByteArray());

  doc::clear_image(img, 0);
  auto& bytes = captured().byteArray();
  ASSERT_EQ(std::size_t(img->getRowStrideSize() * img->height()), bytes.size());
  std::memcpy(img->getPixelAddress(0, 0), bytes.data(), bytes.size());

  EXPECT_EQ(doc::rgba(1, 2, 3, 255), img->getPixel(0, 0));
  EXPECT_EQ(doc::rgba(4, 5, 6, 255), img->getPixel(1, 0));
  EXPECT_EQ(doc::rgba(7, 8, 9, 255), img->getPixel(0, 1));
  EXPECT_EQ(doc::rgba(10, 11, 12, 255), img->getPixel(1, 1));

  // ...and the JS side can write it back too.
  ASSERT_TRUE(eval("var data = native.image.getImageData(); native.image.clear(0);"
                   "native.image.putImageData(data);"));
  EXPECT_EQ(doc::rgba(10, 11, 12, 255), img->getPixel(1, 1));
}

TEST_F(AppScriptApiTest, ImageGetPNGDataReturnsABase64PngDataUri)
{
  // getPNGData() goes through she::instance() (createRgbaSurface,
  // encodeSurfaceAsPNG) - TEST_GUI's ui::Manager deliberately doesn't create
  // a she::System (see tests.h), so this needs its own, scoped to this test.
  std::unique_ptr<she::System> sys(she::create_system());

  useImage(2, 2, doc::rgba(9, 9, 9, 255));

  ASSERT_TRUE(eval("native.capture(native.image.getPNGData());"));

  std::string uri = captured().string();
  const std::string prefix = "data:image/png;base64,";
  ASSERT_GT(uri.size(), prefix.size());
  EXPECT_EQ(prefix, uri.substr(0, prefix.size()));
  EXPECT_GT(uri.size(), prefix.size() + 8) << "should carry actual encoded PNG data, not just the prefix";
}

TEST_F(AppScriptApiTest, WrappedImageHandlesWithTheSameIdCompareEqual)
{
  // The interpreter dedups JS wrappers by the address of the wrapped native
  // object (see QuickJSInterpreter's `wrappers` map), so ScriptRef's
  // per-slot cache (ImageSite::wrap) must hand back the same
  // ScriptRef<doc::Image> instance both times for identity to hold - see
  // issue #232.
  useImage(2, 2, 0);

  ASSERT_TRUE(eval("native.capture(native.image === native.image);"));
  EXPECT_TRUE(captured().boolean());
}

TEST_F(AppScriptApiTest, StaleImageHandleThrowsInsteadOfCrashingAfterFree)
{
  // document.close() now actually frees the underlying Document/Sprite/
  // Layer/Image graph (issue #232), which can leave a script holding a
  // handle into memory that no longer exists (grabbed before the close).
  // Simulate that here directly: free the whole document out from under a JS
  // handle that already resolved it once, the same way DocumentDestroyer
  // frees objects a script may still reference.
  useImage(2, 2, 0);

  ASSERT_TRUE(eval("var stale = native.image;"));

  closeDoc(docs.back());
  TestBridge::site = doc::Site();

  EXPECT_FALSE(eval("stale.getPixel(0, 0);")) << "a handle into a freed object must throw a catchable error, not "
                                                 "dereference freed memory";
  EXPECT_TRUE(eval("1 + 1;")) << "engine must keep working after the throw";
}

TEST_F(AppScriptApiTest, TimersFireFromTick)
{
  ASSERT_TRUE(eval("var fired = false; setTimeout(function() { fired = true; }, 1);"));
  // The timer extension is driven by AppScripting::tick(); give the steady
  // clock a moment to pass the 1ms deadline.
  for (int i = 0; i < 200; ++i)
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    AppScripting::tick();
    ASSERT_TRUE(eval("native.capture(fired);"));
    if (captured().boolean())
      break;
  }
  EXPECT_TRUE(captured().boolean());
}

TEST(ScriptPermissionsTest, LegacyAppLaunchMeansUrlAndOther)
{
  auto path = (std::filesystem::temp_directory_path() / "legacy_app_launch_test.js").string();
  {
    std::ofstream f(path);
    f << "///permission app.launch\n///permission fs.read\n";
  }
  auto perms = app::ScriptPermissions::parseRequested(path);
  std::filesystem::remove(path);
  EXPECT_TRUE(perms.count(app::ScriptPermissions::kAppLaunchUrl));
  EXPECT_EQ(base::launcher::is_available(), perms.count(app::ScriptPermissions::kAppLaunchOther) > 0);
  EXPECT_TRUE(perms.count(app::ScriptPermissions::kFsRead));
  EXPECT_FALSE(perms.count("app.launch"));
}
