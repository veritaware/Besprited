// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

// Phase 1 of the SDL3 migration (see #73, #261): ports window/renderer/
// surface creation & teardown and the SDL2 -> SDL3 return-code convention
// flip. The event loop (key/mouse mapping, SDL_PollEvent translation) and
// tablet/WM-info code are separate phases - SDL3EventQueue below is
// therefore still a stub that never yields real events.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "she/she.h"

#include "base/concurrent_queue.h"
#include "base/exception.h"
#include "base/string.h"
#include "she/sdl3/sdl3_display.h"
#include "she/sdl3/sdl3_surface.h"
#include "she/common/system.h"
#include "she/logger.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include <iostream>
#include <cassert>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <vector>

namespace ui {

  float get_pen_pressure() {
    return 0.0f; // TODO(#73): tablet support.
  }

} // namespace ui

namespace she {

  void log(const std::string&) {
    // TODO(#73)
  }

  namespace sdl {
    bool isMaximized;
    bool isMinimized;
    extern std::unordered_map<int, SDL3Display*> windowIdToDisplay;
  }

  static System* g_instance = nullptr;

  class SDL3EventQueue : public EventQueue {
  public:
    // TODO(#73): translate SDL_Event into she::Event (key/mouse mapping,
    // window events, drag & drop, ...). For now events are drained and
    // dropped so the SDL event queue doesn't grow unbounded.
    void refresh() {
      SDL_PumpEvents();
      SDL_Event sdlEvent;
      while (SDL_PollEvent(&sdlEvent)) {
        // no-op: see TODO above.
      }
    }

    void getEvent(Event& event, bool) override {
      event.setType(Event::None);
    }

    void queueEvent(const Event& event) override {
      m_events.push(event);
    }

  private:
    base::concurrent_queue<Event> m_events;
  };

  EventQueue* EventQueue::instance() {
    static SDL3EventQueue g_queue;
    return &g_queue;
  }

  class SDL3System : public CommonSystem {
  public:
    SDL3System() {
      g_instance = this;
    }

    ~SDL3System() override {
      shutdown = true;
      sleeping = false;
      if (mainThread.joinable())
        mainThread.join();
      SDL_Quit();
      g_instance = nullptr;
    }

    bool shutdown{false};
    std::thread mainThread;
    std::thread::id mainThreadId;
    std::thread::id gfxThreadId;
    std::vector<std::function<void()>> gfxQueue;
    std::atomic<bool> sleeping{false};

    bool isGfxThread() override {
      return std::this_thread::get_id() == gfxThreadId;
    }

    bool isMainThread() override {
      return std::this_thread::get_id() == mainThreadId;
    }

    void gfx(std::function<void()>&& func, bool sleep) override {
      if (isGfxThread()) {
        func();
        return;
      }
      gfxQueue.emplace_back(std::move(func));
      if (sleep)
        this->sleep();
    }

    using Timestamp = std::chrono::high_resolution_clock::time_point;
    Timestamp start = std::chrono::high_resolution_clock::now();

    void sleep() override {
      using namespace std::chrono_literals;
      if (shutdown)
        return;

      if (mainThreadId == gfxThreadId) {
        refresh();
        auto now = std::chrono::high_resolution_clock::now();

        // If the dispatching of messages was faster than 10 milliseconds,
        // it means that the process is not using a lot of CPU, so we can
        // wait the difference to cover those 10 milliseconds
        // sleeping. With this code we can avoid 100% CPU usage.
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        start = now;

        if (elapsed < 15ms)
          std::this_thread::sleep_for(15ms - elapsed);
      } else if (isMainThread()) {
        sleeping = true;
        while (sleeping) {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(10ms);
        }
      }
    }

    std::function<int()> m_func;
    int run(std::function<int()>&& func) override {
      gfxThreadId = std::this_thread::get_id();
      mainThreadId = gfxThreadId;
      return func();
    }

    void refresh() {
      if (!sleeping) {
        static_cast<SDL3EventQueue*>(EventQueue::instance())->refresh();
        return;
      }
      int frames = 5;
      do {
        for (auto& fn : gfxQueue) {
          fn();
        }
        gfxQueue.clear();
        sleeping = false;
        for (auto& entry : sdl::windowIdToDisplay)
          entry.second->present();
        static_cast<SDL3EventQueue*>(EventQueue::instance())->refresh();
      } while (sleeping && --frames);
    }

    void activateApp() override {
      // Do nothing
    }

    void finishLaunching() override {
      // Do nothing
    }

    Capabilities capabilities() const override {
      return (Capabilities)(int(Capabilities::CanResizeDisplay) | int(Capabilities::GpuAccelerationSwitch));
    }

    EventQueue* eventQueue() override { // TODO remove this function
      return EventQueue::instance();
    }

    bool gpuAcceleration() const override {
      return SDL3Display::gpu;
    }

    void setGpuAcceleration(bool state) override {
      if (!unique_display)
        SDL3Display::gpu = state;
    }

    gfx::Size defaultNewDisplaySize() override {
      return {0, 0};
    }

    gfx::Size desktopSize() override {
      // Reports the primary display's resolution in pixels (or in points on
      // platforms where the window is not created high-DPI aware). Returns
      // (0, 0) when SDL can't determine it or the video subsystem isn't up
      // yet.
      if (!SDL_WasInit(SDL_INIT_VIDEO))
        return {0, 0};
      SDL_DisplayID display = SDL_GetPrimaryDisplay();
      if (!display)
        return {0, 0};
      const SDL_DisplayMode* mode = SDL_GetDesktopDisplayMode(display);
      if (!mode)
        return {0, 0};
      return {mode->w, mode->h};
    }

    Display* defaultDisplay() override {
      return unique_display;
    }

    Display* createDisplay(int width, int height, int scale) override {
      //LOG("Creating display %dx%d (scale = %d)\n", width, height, scale);
      return new SDL3Display(width, height, scale);
    }

    Surface* createSurface(int width, int height) override {
      return new SDL3Surface(width, height, SDL3Surface::DeleteAndDestroy);
    }

    Surface* createRgbaSurface(int width, int height) override {
      return new SDL3Surface(width, height, 32, SDL3Surface::DeleteAndDestroy);
    }

    std::vector<uint8_t> encodeSurfaceAsPNG(Surface* s) override {
      auto surface = static_cast<SDL3Surface*>(s);
      std::vector<uint8_t> data;
      data.resize(surface->width() * surface->height() * 4 + 1024);
      const std::shared_ptr<SDL_IOStream> rops{
        SDL_IOFromMem(data.data(), data.size()),
        [](auto *rops){ SDL_CloseIO(rops); }
      };
      if (!IMG_SavePNG_IO(static_cast<SDL_Surface*>(surface->nativeHandle()), rops.get(), false))
        return {};
      data.resize(SDL_TellIO(rops.get()));
      return data;
    }

    Surface* loadSurface(const char* filename) override {
      SDL_Surface* bmp = IMG_Load(filename);
      if (!bmp)
        throw std::runtime_error(std::string{"Error loading image "} + filename);
      return new SDL3Surface(bmp, SDL3Surface::DeleteAndDestroy);
    }

    Surface* loadRgbaSurface(const char* filename) override {
      SDL_Surface* bmp = IMG_Load(filename);
      if (!bmp)
        throw std::runtime_error(std::string{"Error loading image "} + filename);
      const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(bmp->format);
      if (details && details->bits_per_pixel < 32) {
        auto copy = SDL_ConvertSurface(bmp, SDL_PIXELFORMAT_RGBA8888);
        if (!copy)
          throw std::runtime_error(std::string{"Error converting image "} + filename);
        SDL_DestroySurface(bmp);
        bmp = copy;
      }
      return new SDL3Surface(bmp, SDL3Surface::DeleteAndDestroy);
    }

  };

  System* create_system() {
    return new SDL3System();
  }

  System* instance() {
    return g_instance;
  }

  void error_message(const char* msg) {
    if (g_instance && g_instance->logger())
      g_instance->logger()->logError(msg);
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, PACKAGE, msg, nullptr);
  }

  int scancode_to_ascii(KeyScancode) {
    return 0; // TODO(#73): key mapping is ported with the event loop.
  }

  bool is_key_pressed(KeyScancode) {
    return false; // TODO(#73)
  }

  void clear_keyboard_buffer() {
    // TODO(#73)
  }

  void set_input_rect(const gfx::Rect&) {
    // TODO(#73): SDL_StartTextInput()/SDL_StopTextInput()/
    // SDL_SetTextInputArea() take an SDL_Window* in SDL3 (SDL2 was
    // global); ported together with the event loop in a later phase.
  }

} // namespace she

// It must be defined by the user program code.
extern int app_main(int argc, char* argv[]);

int main(const int argc, char* argv[]) {
  #ifdef SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR
  SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
  #endif
  SDL_SetHint(SDL_HINT_VIDEO_ALLOW_SCREENSAVER, "1");

  // SDL_WINDOWS_DPI_AWARENESS was removed in SDL3: it is always
  // per-monitor-v2 aware on Windows now (see #261).

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    std::cerr << "Critical: Could not initialize SDL3 (" << SDL_GetError() << "). Aborting.\n";
    return -1;
  }
  // IMG_Init()/IMG_Quit() were removed in SDL3_image: loaders are
  // initialized/torn down automatically as needed.
  return app_main(argc, argv);
}
