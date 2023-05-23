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
    Scroll
  };

  class TK_API Event
  {
   public:
    enum class EventType
    {
      Null,
      Mouse,
      Keyboard
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

} // namespace ToolKit