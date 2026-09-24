// SHE library
// Copyright (C) 2026 Veritaware
//
// This file is released under the terms of the MIT license.
// Read LICENSE.txt for more information.

// Phase 2 of the SDL3 migration (see #73, #262): ports the event loop
// (SDL_PollEvent translation, key/mouse mapping) on top of Phase 1's
// window/renderer/surface port. SDL_SYSWMEVENT and tablet/WM-info code
// (EasyTab, nativeHandle()) are still a separate phase.

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
#include <deque>
#include <thread>
#include <unordered_map>
#include <vector>

float penPressure = 0;

namespace ui {
  float get_pen_pressure() {
    return penPressure;
  }
}

static she::System* g_instance = nullptr;
static std::unordered_map<int, she::Event::MouseButton> mouseButtonMapping = {
  {SDL_BUTTON_LEFT, she::Event::LeftButton},
  {SDL_BUTTON_MIDDLE, she::Event::MiddleButton},
  {SDL_BUTTON_RIGHT, she::Event::RightButton}
};
static she::KeyScancode lastScancode;
static int lastScancodeSDL;
struct Modifier {
  const int sheModifier;
  int ascii;
  bool isPressed = false;
  Modifier(int sheModifier) : sheModifier(sheModifier) {}
};

static std::unordered_map<int, Modifier*> reverseKeyCodeMapping;

static std::unordered_map<SDL_Keycode, Modifier> keyCodeMapping = {
  {SDLK_UNKNOWN, she::kKeyNil},
  {static_cast<SDL_Keycode>(13), she::kKeyEnter},
  {SDLK_PERIOD, she::kKeyStop},
  {SDLK_A, she::kKeyA},
  {SDLK_B, she::kKeyB},
  {SDLK_C, she::kKeyC},
  {SDLK_D, she::kKeyD},
  {SDLK_E, she::kKeyE},
  {SDLK_F, she::kKeyF},
  {SDLK_G, she::kKeyG},
  {SDLK_H, she::kKeyH},
  {SDLK_I, she::kKeyI},
  {SDLK_J, she::kKeyJ},
  {SDLK_K, she::kKeyK},
  {SDLK_L, she::kKeyL},
  {SDLK_M, she::kKeyM},
  {SDLK_N, she::kKeyN},
  {SDLK_O, she::kKeyO},
  {SDLK_P, she::kKeyP},
  {SDLK_Q, she::kKeyQ},
  {SDLK_R, she::kKeyR},
  {SDLK_S, she::kKeyS},
  {SDLK_T, she::kKeyT},
  {SDLK_U, she::kKeyU},
  {SDLK_V, she::kKeyV},
  {SDLK_W, she::kKeyW},
  {SDLK_X, she::kKeyX},
  {SDLK_Y, she::kKeyY},
  {SDLK_Z, she::kKeyZ},
  {SDLK_0, she::kKey0},
  {SDLK_1, she::kKey1},
  {SDLK_2, she::kKey2},
  {SDLK_3, she::kKey3},
  {SDLK_4, she::kKey4},
  {SDLK_5, she::kKey5},
  {SDLK_6, she::kKey6},
  {SDLK_7, she::kKey7},
  {SDLK_8, she::kKey8},
  {SDLK_9, she::kKey9},
  {SDLK_KP_0, she::kKey0Pad},
  {SDLK_KP_1, she::kKey1Pad},
  {SDLK_KP_2, she::kKey2Pad},
  {SDLK_KP_3, she::kKey3Pad},
  {SDLK_KP_4, she::kKey4Pad},
  {SDLK_KP_5, she::kKey5Pad},
  {SDLK_KP_6, she::kKey6Pad},
  {SDLK_KP_7, she::kKey7Pad},
  {SDLK_KP_8, she::kKey8Pad},
  {SDLK_KP_9, she::kKey9Pad},
  {SDLK_F1, she::kKeyF1},
  {SDLK_F2, she::kKeyF2},
  {SDLK_F3, she::kKeyF3},
  {SDLK_F4, she::kKeyF4},
  {SDLK_F5, she::kKeyF5},
  {SDLK_F6, she::kKeyF6},
  {SDLK_F7, she::kKeyF7},
  {SDLK_F8, she::kKeyF8},
  {SDLK_F9, she::kKeyF9},
  {SDLK_F10, she::kKeyF10},
  {SDLK_F11, she::kKeyF11},
  {SDLK_F12, she::kKeyF12},
  {SDLK_ESCAPE, she::kKeyEsc},
  {SDLK_APOSTROPHE, she::kKeyTilde},
  {SDLK_MINUS, she::kKeyMinus},
  {SDLK_EQUALS, she::kKeyEquals},
  {SDLK_BACKSPACE, she::kKeyBackspace},
  {SDLK_TAB, she::kKeyTab},
  {SDLK_LEFTBRACKET, she::kKeyOpenbrace},
  {SDLK_RIGHTBRACKET, she::kKeyClosebrace},
  {SDLK_KP_ENTER, she::kKeyEnter},
  {SDLK_COLON, she::kKeyColon},
  {SDLK_APOSTROPHE, she::kKeyQuote},
  {SDLK_BACKSLASH, she::kKeyBackslash},
  // {SDLK_BACKSLASH2, she::kKeyBackslash2},
  {SDLK_COMMA, she::kKeyComma},
  {SDLK_STOP, she::kKeyStop},
  {SDLK_SLASH, she::kKeySlash},
  {SDLK_SPACE, she::kKeySpace},
  {SDLK_INSERT, she::kKeyInsert},
  {SDLK_DELETE, she::kKeyDel},
  {SDLK_HOME, she::kKeyHome},
  {SDLK_END, she::kKeyEnd},
  {SDLK_PAGEUP, she::kKeyPageUp},
  {SDLK_PAGEDOWN, she::kKeyPageDown},
  {SDLK_LEFT, she::kKeyLeft},
  {SDLK_RIGHT, she::kKeyRight},
  {SDLK_UP, she::kKeyUp},
  {SDLK_DOWN, she::kKeyDown},
  {SDLK_KP_DIVIDE, she::kKeySlashPad},
  {SDLK_ASTERISK, she::kKeyAsterisk},
  {SDLK_KP_MINUS, she::kKeyMinusPad},
  {SDLK_KP_PLUS, she::kKeyPlusPad},
  // {SDLK_KP_DEL, she::kKeyDelPad},
  {SDLK_KP_PERIOD, she::kKeyDelPad},
  {SDLK_KP_ENTER, she::kKeyEnterPad},
  {SDLK_PRINTSCREEN, she::kKeyPrtscr},
  {SDLK_PAUSE, she::kKeyPause},
  // {SDLK_ABNTC1, she::kKeyAbntC1},
  // {SDLK_YEN, she::kKeyYen},
  // {SDLK_KANA, she::kKeyKana},
  // {SDLK_CONVERT, she::kKeyConvert},
  // {SDLK_NOCONVERT, she::kKeyNoconvert},
  {SDLK_AT, she::kKeyAt},
  // {SDLK_CIRCUMFLEX, she::kKeyCircumflex},
  // {SDLK_COLON2, she::kKeyColon2},
  // {SDLK_KANJI, she::kKeyKanji},
  {SDLK_KP_EQUALS, she::kKeyEqualsPad},
  {SDLK_GRAVE, she::kKeyBackquote},
  {SDLK_SEMICOLON, she::kKeySemicolon},
  // {SDLK_COMMAND, she::kKeyCommand},
  // {SDLK_UNKNOWN1, she::kKeyUnknown1},
  // {SDLK_UNKNOWN2, she::kKeyUnknown2},
  // {SDLK_UNKNOWN3, she::kKeyUnknown3},
  // {SDLK_UNKNOWN4, she::kKeyUnknown4},
  // {SDLK_UNKNOWN5, she::kKeyUnknown5},
  // {SDLK_UNKNOWN6, she::kKeyUnknown6},
  // {SDLK_UNKNOWN7, she::kKeyUnknown7},
  // {SDLK_UNKNOWN8, she::kKeyUnknown8},
  {SDLK_LSHIFT, she::kKeyLShift},
  {SDLK_RSHIFT, she::kKeyRShift},
  {SDLK_LCTRL, she::kKeyLControl},
  {SDLK_RCTRL, she::kKeyRControl},
  {SDLK_LALT, she::kKeyAlt},
  {SDLK_RALT, she::kKeyAltGr},
  {SDLK_LGUI, she::kKeyLWin},
  // {SDLK_RWIN, she::kKeyRWin},
  {SDLK_MENU, she::kKeyMenu},
  {SDLK_SCROLLLOCK, she::kKeyScrLock},
  {SDLK_NUMLOCKCLEAR, she::kKeyNumLock},
  {SDLK_CAPSLOCK, she::kKeyCapsLock},
};

std::unordered_map<SDL_Keycode, Modifier> modifiers = {
  {SDLK_SPACE, she::kKeySpaceModifier},

  {SDLK_LALT, she::kKeyAltModifier},
  {SDLK_RALT, she::kKeyAltModifier},

  {SDLK_LCTRL, she::kKeyCtrlModifier},
  {SDLK_RCTRL, she::kKeyCtrlModifier},

  {SDLK_LGUI, she::kKeyCmdModifier},
  {SDLK_RGUI, she::kKeyCmdModifier},

  {SDLK_LSHIFT, she::kKeyShiftModifier},
  {SDLK_RSHIFT, she::kKeyShiftModifier}
};

she::KeyModifiers getSheModifiers() {
  int mod = 0;
  for (auto& entry : modifiers) {
    if (entry.second.isPressed)
      mod |= entry.second.sheModifier;
  }
  return static_cast<she::KeyModifiers>(mod);
}

static std::deque<she::Event> keybuffer;
static bool display_has_mouse = false;

namespace she {

  void log(const std::string&) {
    // TODO(#73)
  }

  namespace sdl {
    bool isMaximized;
    bool isMinimized;
    extern std::unordered_map<int, SDL3Display*> windowIdToDisplay;
  }

  class SDL3EventQueue : public EventQueue {
  public:
    PointerType pointerType = PointerType::Mouse;
    std::chrono::steady_clock::time_point lastUpTime = std::chrono::steady_clock::now();

    SDL3EventQueue() {
      if (reverseKeyCodeMapping.empty()) {
        for (auto& entry : keyCodeMapping) {
          reverseKeyCodeMapping[entry.second.sheModifier] = &entry.second;
          entry.second.ascii = entry.first;
        }
      }
    }

    void forceFlip() {
      for (auto& entry : sdl::windowIdToDisplay) {
        entry.second->flip({
            0,
            0,
            entry.second->width(),
            entry.second->height()
          });
        entry.second->present();
      }
    }

    void refresh() {
      if (!m_events.empty())
        return;
      Event event;
      while (true) {
        event.setType(Event::None);
        getEventInternal(event, false);
        if (event.type() == Event::None) {
          return;
        }
        m_events.push(event);
      }
    }

    void getEvent(Event& event, bool) override {
      event.setType(Event::None);
      if (m_events.try_pop(event))
        return;
      if (she::instance()->isGfxThread())
        getEventInternal(event, false);
    }

    void getEventInternal(Event& event, bool) {
      SDL_Event sdlEvent;
      while (SDL_PollEvent(&sdlEvent)) {
        switch (sdlEvent.type) {
        case SDL_EVENT_DID_ENTER_FOREGROUND:
          SDL3Surface::textureGen++;
          forceFlip();
          continue;

        // SDL_SYSWMEVENT has no SDL3 equivalent (SDL_SetWindowsMessageHook /
        // SDL_SetX11EventHook replace it); tablet WM-info handling moves
        // there - see #73 (tablet/WM-info phase).

        case SDL_EVENT_WINDOW_EXPOSED:
          forceFlip();
          continue;

        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
          continue;

        case SDL_EVENT_WINDOW_MAXIMIZED:
          sdl::isMaximized = true;
          sdl::isMinimized = false;
          std::cout << "Maximized" << "\n";
          continue;

        case SDL_EVENT_WINDOW_MINIMIZED:
          sdl::isMaximized = false;
          sdl::isMinimized = true;
          std::cout << "Minimized" << "\n";
          continue;

        case SDL_EVENT_WINDOW_RESTORED:
          sdl::isMaximized = false;
          sdl::isMinimized = false;
          std::cout << "Restored" << "\n";
          continue;

        case SDL_EVENT_WINDOW_RESIZED: {
          auto it = sdl::windowIdToDisplay.find(static_cast<int>(sdlEvent.window.windowID));
          if (it == sdl::windowIdToDisplay.end())
            continue;
          auto display = it->second;
          display->setWidth(sdlEvent.window.data1);
          display->setHeight(sdlEvent.window.data2);
          display->recreateSurface();
          event.setType(Event::ResizeDisplay);
          event.setDisplay(display);
          return;
        }

        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
          if (display_has_mouse) {
            display_has_mouse = false;

            Event ev;
            ev.setType(Event::MouseLeave);
            m_events.push(ev);
          }
          continue;

        case SDL_EVENT_WINDOW_FOCUS_LOST:
          // Alt-Tabbing away from the window doesn't deliver the matching
          // key-up, so without this the modifier gets stuck "pressed".
          for (auto& entry : modifiers)
            entry.second.isPressed = false;
          continue;

        // Silence 'Unknown event' console spam for common/frequent SDL3
        // window events with nothing for us to react to yet.
        case SDL_EVENT_WINDOW_SHOWN:
        case SDL_EVENT_WINDOW_HIDDEN:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_METAL_VIEW_RESIZED:
        case SDL_EVENT_WINDOW_HIT_TEST:
        case SDL_EVENT_WINDOW_ICCPROF_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        case SDL_EVENT_WINDOW_DISPLAY_SCALE_CHANGED:
        case SDL_EVENT_WINDOW_SAFE_AREA_CHANGED:
        case SDL_EVENT_WINDOW_OCCLUDED:
        case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
        case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
        case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
        case SDL_EVENT_WINDOW_DESTROYED:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
          // Closing the app is handled elsewhere so we can ignore it here.
          continue;

        case SDL_EVENT_MOUSE_MOTION:
          if (!display_has_mouse) {
            display_has_mouse = true;
            Event ev;
            ev.setType(Event::MouseEnter);
            m_events.push(ev);
          }

          // Drain excess SDL_EVENT_MOUSE_MOTION and SDL_EVENT_FINGER_MOTION
          // events, keeping only the most recent.
          {
            SDL_Event nextEvent;
            while (SDL_PeepEvents(&nextEvent, 1, SDL_GETEVENT, SDL_EVENT_MOUSE_MOTION, SDL_EVENT_MOUSE_MOTION) > 0) {
              sdlEvent = nextEvent;
            }
            while (SDL_PeepEvents(&nextEvent, 1, SDL_GETEVENT, SDL_EVENT_FINGER_MOTION, SDL_EVENT_FINGER_MOTION) > 0) {
              penPressure = std::max(nextEvent.tfinger.pressure, 0.0001f);
            }
          }

          event.setType(Event::MouseMove);
          event.setModifiers(getSheModifiers());
          event.setPosition({
              static_cast<int>(sdlEvent.motion.x / unique_display->scale()),
              static_cast<int>(sdlEvent.motion.y / unique_display->scale())
            });

          {
              const int hasFingerEvent = SDL_PeepEvents(&sdlEvent, 1, SDL_PEEKEVENT, SDL_EVENT_FINGER_MOTION, SDL_EVENT_FINGER_MOTION);
              if (hasFingerEvent) {
                  penPressure = std::max<>(sdlEvent.tfinger.pressure, 0.0001f);
              }
          }


          event.setPressure(penPressure);
          event.setPointerType(pointerType);
          return;

        case SDL_EVENT_FINGER_MOTION:
          penPressure = std::max<>(sdlEvent.tfinger.pressure, 0.0001f);
          continue;

        case SDL_EVENT_MOUSE_WHEEL:
          event.setType(Event::MouseWheel);
          event.setModifiers(getSheModifiers());
          event.setWheelDelta({-sdlEvent.wheel.integer_x, -sdlEvent.wheel.integer_y});
          {
            float x, y;
            SDL_GetMouseState(&x, &y);
            event.setPosition({
                static_cast<int>(x / unique_display->scale()),
                static_cast<int>(y / unique_display->scale())
              });
          }
          return;

        case SDL_EVENT_MOUSE_BUTTON_UP:
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
          auto type = sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? Event::MouseDown : Event::MouseUp;
          event.setType(type);
          event.setPosition({
              static_cast<int>(sdlEvent.button.x / unique_display->scale()),
              static_cast<int>(sdlEvent.button.y / unique_display->scale())
            });
          event.setButton(mouseButtonMapping[sdlEvent.button.button]);
          event.setModifiers(getSheModifiers());

          if (penPressure > 0.0f) {
            pointerType = PointerType::Pen;
            event.setPressure(penPressure);
            event.setPointerType(pointerType);
          } else {
            event.setPressure(sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_DOWN ? 1.0f : 0.0f);
            event.setPointerType(pointerType);
            pointerType = PointerType::Mouse;
          }

          auto now = std::chrono::steady_clock::now();
          auto delta = now - lastUpTime;
          if (sdlEvent.type == SDL_EVENT_MOUSE_BUTTON_UP) {
            using namespace std::chrono_literals;
            if (delta < 200ms) {
              m_events.push(event);
              event.setType(Event::MouseDoubleClick);
              event.setPosition(event.position());
              event.setButton(event.button());
            }
            lastUpTime = now;
          }

          return;
        }

        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP: {
          Event event;
          const bool isPressed = sdlEvent.type == SDL_EVENT_KEY_DOWN;
          auto modifierIt = modifiers.find(sdlEvent.key.key);
          if (modifierIt != modifiers.end()) {
            modifierIt->second.isPressed = isPressed;
          }

          auto it = keyCodeMapping.find(sdlEvent.key.key);

          if (it == keyCodeMapping.end()) {
            std::cout << "Unknown scancode: " << sdlEvent.key.key << "\n";
            continue;
          }

          event.setType(isPressed ? Event::KeyDown : Event::KeyUp);
          auto modifiers = getSheModifiers();
          event.setModifiers(modifiers);
          it->second.isPressed = isPressed;
          auto scancode = static_cast<she::KeyScancode>(it->second.sheModifier);
          event.setScancode(scancode);
          if (isPressed) {
            lastScancode = scancode;
            lastScancodeSDL = sdlEvent.key.scancode;
          }
          if (sdlEvent.key.repeat) {
            event.setRepeat(sdlEvent.key.repeat);
          }
          keybuffer.push_back(event);
          if (modifiers & (she::kKeyCtrlModifier | she::kKeyCmdModifier)) {
            if (unique_display)
              SDL_StopTextInput(unique_display->window());
            break;
          } else if (unique_display && !SDL_TextInputActive(unique_display->window())) {
            SDL_StartTextInput(unique_display->window());
          }
          continue;
        }

        case SDL_EVENT_DROP_FILE: {
          const std::string file(sdlEvent.drop.data);
          event.setType(Event::DropFiles);
          event.setFiles({file});
          // sdlEvent.drop.data is owned by SDL and freed with the event in
          // SDL3 - unlike SDL2's drop.file, it must NOT be SDL_free()'d here.
          return;
        }

        case SDL_EVENT_DROP_BEGIN:
        case SDL_EVENT_DROP_COMPLETE:
        case SDL_EVENT_DROP_POSITION:
        case SDL_EVENT_DROP_TEXT:
          continue;

          // CloseDisplay,
          // ResizeDisplay,
          // MouseEnter,
          // MouseLeave,
          // TouchMagnify,
        case SDL_EVENT_QUIT:
          event.setType(Event::CloseDisplay);
          return;

        case SDL_EVENT_TEXT_EDITING:
          continue;

        case SDL_EVENT_TEXT_INPUT: {
          keybuffer.clear();
          std::string textString = sdlEvent.text.text;
          const base::utf8_const_iterator begin{textString.begin()};
          const base::utf8_const_iterator end{textString.end()};
          Event event;
          event.setModifiers(getSheModifiers());
          for (auto it = begin; it != end; ++it) {
            event.setType(Event::KeyDown);
            event.setUnicodeChar(*it);
            if (lastScancodeSDL > SDL_SCANCODE_UNKNOWN && lastScancodeSDL < SDL_SCANCODE_RETURN) {
              event.setScancode(lastScancode);
              lastScancodeSDL = SDL_SCANCODE_UNKNOWN;
            }
            keybuffer.push_back(event);
            event.setType(Event::KeyUp);
            keybuffer.push_back(event);
          }

          break;
        }

        case SDL_EVENT_KEYMAP_CHANGED:
        case SDL_EVENT_KEYBOARD_ADDED:
        case SDL_EVENT_KEYBOARD_REMOVED:
        case SDL_EVENT_MOUSE_ADDED:
        case SDL_EVENT_MOUSE_REMOVED:
        case SDL_EVENT_CLIPBOARD_UPDATE:
          continue;

        default:
          if (sdlEvent.type >= SDL_EVENT_WINDOW_FIRST && sdlEvent.type <= SDL_EVENT_WINDOW_LAST)
            continue; // Unhandled future SDL3 window event - don't spam.
          if (sdlEvent.type >= SDL_EVENT_DISPLAY_FIRST && sdlEvent.type <= SDL_EVENT_DISPLAY_LAST)
            continue; // Display hotplug/mode-change events - not acted on yet.
          std::cout << "Unknown event: " << static_cast<int>(sdlEvent.type) << "\n";
          continue;
        }
      }

      if (!keybuffer.empty()) {
        event = keybuffer.front();
        keybuffer.pop_front();
        return;
      }
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

  int scancode_to_ascii(KeyScancode scancode) {
    auto it = reverseKeyCodeMapping.find(scancode);
    if (it == reverseKeyCodeMapping.end())
      return 0;
    return it->second->ascii;
  }

  bool is_key_pressed(KeyScancode scancode) {
    auto it = reverseKeyCodeMapping.find(scancode);
    if (it != reverseKeyCodeMapping.end()) {
      return it->second->isPressed;
    }
    return false;
  }

  void set_input_rect(const gfx::Rect& rect) {
    if (!unique_display)
      return;
    auto window = unique_display->window();
    if (rect.isEmpty()) {
      SDL_StopTextInput(window);
      return;
    }
    const SDL_Rect sdlRect{
      .x = rect.x,
      .y = rect.y,
      .w = rect.w,
      .h = rect.h
    };
    SDL_SetTextInputArea(window, &sdlRect, 0);
    SDL_StartTextInput(window);
  }

  void clear_keyboard_buffer() {
    keybuffer.clear();
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

  // SDL3 applies Shift/Caps Lock to the reported keycode by default (e.g.
  // Shift+1 yields SDLK_EXCLAIM, not SDLK_1), which keyCodeMapping doesn't
  // know about and would silently drop. "unmodified" restores SDL2's
  // behavior of reporting the base, unshifted keycode (see #262).
  SDL_SetHint(SDL_HINT_KEYCODE_OPTIONS, "unmodified");

  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
    std::cerr << "Critical: Could not initialize SDL3 (" << SDL_GetError() << "). Aborting.\n";
    return -1;
  }
  // IMG_Init()/IMG_Quit() were removed in SDL3_image: loaders are
  // initialized/torn down automatically as needed.
  SDL_SetEventEnabled(SDL_EVENT_FINGER_MOTION, true);
  return app_main(argc, argv);
}
