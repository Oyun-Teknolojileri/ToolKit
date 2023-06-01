/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
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
  };

  class TK_API Event
  {
   public:
    enum class EventType
    {
      Null,
      Mouse,
      Keyboard,
      Gamepad
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

  enum class GamepadButton : uint
  {
    None          = 0,
    A             = 1 << 0, Circle   = 1 << 0, //!< (PS) Circle   () =  (Xbox) A 
    B             = 1 << 1, Cross    = 1 << 1, //!< (PS) Cross    X  =  (Xbox) B 
    X             = 1 << 2, Triangle = 1 << 2, //!< (PS) Triangle /\ =  (Xbox) X  
    Y             = 1 << 3, Square   = 1 << 3, //!< (PS) Square   [] =  (Xbox) Y 
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
    Misc1         = 1 << 15, //!< Xbox Series X share button, PS5 microphone button, Nintendo Switch Pro capture button, Amazon Luna microphone button
    Paddle1       = 1 << 16, //!< Xbox Elite paddle P1 
    Paddle2       = 1 << 17, //!< Xbox Elite paddle P3 
    Paddle3       = 1 << 18, //!< Xbox Elite paddle P2 
    Paddle4       = 1 << 19, //!< Xbox Elite paddle P4 
    Touchpad      = 1 << 20, //!< PS4/PS5 touchpad button
    MaxBit        = 1 << 21, //!< You can use when you iterate through bits
    Count         = 21
  };

  class TK_API GamepadEvent : public Event
  {
   public:
    GamepadEvent() { m_type = EventType::Gamepad; }
    
    enum class StickAxis
    {
      LeftX       , 
      LeftY       , 
      RightX      , 
      RightY      , 
      TriggerLeft , 
      TriggerRight
    };

    float m_angle;
    StickAxis m_axis;
    GamepadButton m_button;
  };

} // namespace ToolKit

