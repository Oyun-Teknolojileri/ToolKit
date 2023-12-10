/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

namespace ToolKit
{

  enum class EventAction
  {
    Null,
    KeyDown,
    KeyUp,
    LeftClick,
    RightClick,
    MiddleClick,
    Move,
    Scroll,
    GamepadAxis,
    GamepadButtonDown,
    GamepadButtonUp,
    Touch
  };

  class TK_API Event
  {
   public:
    enum class EventType
    {
      Null,
      Mouse,
      Keyboard,
      Gamepad,
      Touch
    };

    EventType m_type     = EventType::Null;
    EventAction m_action = EventAction::Null;
  };

  class TK_API KeyboardEvent : public Event
  {
   public:
    KeyboardEvent() { m_type = EventType::Keyboard; }

   public:
    int m_keyCode = 0;
    int m_mode    = 0; // Or combination of key modifiers.
  };

  class TK_API MouseEvent : public Event
  {
   public:
    MouseEvent() { m_type = EventType::Mouse; }

   public:
    bool m_release  = false;  // If true mouse button is up else down.
    int absolute[2] = {0, 0}; // x, y.
    int relative[2] = {0, 0}; // x, y.
    int scroll[2]   = {0, 0}; // x, y.
  };

  class TK_API TouchEvent : public Event
  {
   public:
    TouchEvent() { m_type = EventType::Touch; }

   public:
    bool m_release  = false;
    float absolute[2] = {0.0f, 0.0f}; // x, y. Between 0-1
  };

  enum class GamepadButton : uint
  {
    None          = 0,
    A             = 1 << 0,
    Cross         = 1 << 0, //!< (PS) Cross    X  =  (Xbox) A
    B             = 1 << 1,
    Circle        = 1 << 1, //!< (PS) Circle   () =  (Xbox) B
    Y             = 1 << 2,
    Square        = 1 << 2, //!< (PS) Square   [] =  (Xbox) Y
    X             = 1 << 3,
    Triangle      = 1 << 3, //!< (PS) Triangle /\ =  (Xbox) X
    Back          = 1 << 4, //!< Select
    Guide         = 1 << 5, //!< Mode
    Start         = 1 << 6,
    LeftStick     = 1 << 7,
    RightStick    = 1 << 8,
    LeftShoulder  = 1 << 9,  //!< L1
    RightShoulder = 1 << 10, //!< R1
    DpadUp        = 1 << 11,
    DpadDown      = 1 << 12,
    DpadLeft      = 1 << 13,
    DpadRight     = 1 << 14,
    Misc1 = 1 << 15, //!< Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon
                     //!< Luna microphone button
    Paddle1  = 1 << 16, //!< Xbox Elite paddle P1
    Paddle2  = 1 << 17, //!< Xbox Elite paddle P3
    Paddle3  = 1 << 18, //!< Xbox Elite paddle P2
    Paddle4  = 1 << 19, //!< Xbox Elite paddle P4
    Touchpad = 1 << 20, //!< PS4/PS5 touchpad button
    MaxBit   = 1 << 21, //!< You can use when you iterate through bits
    Count    = 21
  };

  class TK_API GamepadEvent : public Event
  {
   public:
    GamepadEvent() { m_type = EventType::Gamepad; }

    enum class StickAxis
    {
      LeftX,
      LeftY,
      RightX,
      RightY,
      TriggerLeft,
      TriggerRight
    };

    float m_angle;
    StickAxis m_axis;
    GamepadButton m_button;
  };

} // namespace ToolKit
