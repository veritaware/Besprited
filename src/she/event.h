// SHE Library
// Aseprite  | Copyright (C) 2012-2016 David Capello
// Besprited | Copyright (C) 2026      Veritaware
//
// This source file is ditributed under a BSD-like license, please
// read LICENSE.txt for more information.

#pragma once

#include "gfx/point.h"
#include "gfx/size.h"
#include "she/keys.h"
#include "she/pointer_type.h"

#include <cstdint>
#include <string>
#include <vector>

#pragma push_macro("None")
#undef None // Undefine the X11 None macro

namespace she
{

class Display;

class Event
{
public:
  enum Type : std::uint8_t
  {
    None,
    CloseDisplay,
    ResizeDisplay,
    DropFiles,
    MouseEnter,
    MouseLeave,
    MouseMove,
    MouseDown,
    MouseUp,
    MouseWheel,
    MouseDoubleClick,
    KeyDown,
    KeyUp,
    TouchMagnify,
  };

  enum MouseButton : std::uint8_t
  {
    NoneButton,
    LeftButton,
    RightButton,
    MiddleButton
  };

  using Files = std::vector<std::string>;

  Event() = default;

  [[nodiscard]] Type type() const { return m_type; }
  [[nodiscard]] Display* display() const { return m_display; }
  [[nodiscard]] const Files& files() const { return m_files; }
  [[nodiscard]] KeyScancode scancode() const { return m_scancode; }
  [[nodiscard]] KeyModifiers modifiers() const { return m_modifiers; }
  [[nodiscard]] int unicodeChar() const { return m_unicodeChar; }
  [[nodiscard]] int repeat() const { return m_repeat; }
  [[nodiscard]] gfx::Point position() const { return m_position; }
  [[nodiscard]] gfx::Point wheelDelta() const { return m_wheelDelta; }

  // We suppose that if we are receiving precise scrolling deltas,
  // it means that the user is using a touch-like surface (trackpad,
  // magic mouse scrolling, touch wacom tablet, etc.)
  // TODO change this with the new PointerType::Multitouch
  [[nodiscard]] bool preciseWheel() const { return m_preciseWheel; }

  [[nodiscard]] PointerType pointerType() const { return m_pointerType; }
  [[nodiscard]] MouseButton button() const { return m_button; }
  [[nodiscard]] double magnification() const { return m_magnification; }
  [[nodiscard]] double pressure() const { return m_pressure; }

  void setType(Type type) { m_type = type; }
  void setDisplay(Display* display) { m_display = display; }
  void setFiles(const Files& files) { m_files = files; }

  void setScancode(KeyScancode scancode) { m_scancode = scancode; }
  void setModifiers(KeyModifiers modifiers) { m_modifiers = modifiers; }
  void setUnicodeChar(int unicodeChar) { m_unicodeChar = unicodeChar; }
  void setRepeat(int repeat) { m_repeat = repeat; }
  void setPosition(const gfx::Point& pos) { m_position = pos; }
  void setWheelDelta(const gfx::Point& delta) { m_wheelDelta = delta; }
  void setPreciseWheel(bool precise) { m_preciseWheel = precise; }
  void setPointerType(PointerType pointerType) { m_pointerType = pointerType; }
  void setButton(MouseButton button) { m_button = button; }
  void setMagnification(double magnification)
  {
    m_magnification = magnification;
  }
  void setPressure(double pressure) { m_pressure = pressure; }

private:
  Type m_type = None;
  Display* m_display = nullptr;
  Files m_files;
  KeyScancode m_scancode = kKeyNil;
  KeyModifiers m_modifiers = kKeyUninitializedModifier;
  int m_unicodeChar = 0;
  int m_repeat = 0; // repeat=0 means the first time the key is pressed
  gfx::Point m_position;
  gfx::Point m_wheelDelta;
  bool m_preciseWheel = false;
  PointerType m_pointerType = PointerType::Unknown;
  MouseButton m_button = NoneButton;

  // For TouchMagnify event
  double m_magnification = 0.0;

  // Pressure of stylus used in mouse-like events
  double m_pressure = 0.0;
};

} // namespace she

#pragma pop_macro("None")
