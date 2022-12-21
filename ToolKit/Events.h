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