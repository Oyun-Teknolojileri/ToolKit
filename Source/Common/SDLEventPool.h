#pragma once

#include "SDL.h"
#include "ToolKit.h"

namespace ToolKit
{
  // Utility functions for Pooling & Releaseing SDL events for ToolKit.
  void PoolEvent(const SDL_Event& e)
  {
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
    {
      MouseEvent* me = new MouseEvent();
      Main::GetInstance()->m_eventPool.push_back(me);

      if (e.type == SDL_MOUSEBUTTONDOWN)
      {
        me->m_release = false;
      }
      else
      {
        me->m_release = true;
      }

      switch (e.button.button)
      {
      case SDL_BUTTON_LEFT:
        me->m_action = EventAction::LeftClick;
        break;
      case SDL_BUTTON_RIGHT:
        me->m_action = EventAction::RightClick;
        break;
      case SDL_BUTTON_MIDDLE:
        me->m_action = EventAction::MiddleClick;
        break;
      }
    }

    if (e.type == SDL_MOUSEMOTION)
    {
      MouseEvent* me = new MouseEvent();
      Main::GetInstance()->m_eventPool.push_back(me);

      me->m_action = EventAction::Move;
      me->absolute[0] = e.motion.x;
      me->absolute[1] = e.motion.y;
      me->relative[0] = e.motion.xrel;
      me->relative[1] = e.motion.yrel;
    }

    if (e.type == SDL_MOUSEWHEEL)
    {
      MouseEvent* me = new MouseEvent();
      Main::GetInstance()->m_eventPool.push_back(me);

      me->m_action = EventAction::Scroll;
      me->scroll[0] = e.wheel.x;
      me->scroll[1] = e.wheel.y;
    }

    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
    {
      SDL_Keymod modState = SDL_GetModState();

      KeyboardEvent* ke = new KeyboardEvent();
      Main::GetInstance()->m_eventPool.push_back(ke);

      if (e.type == SDL_KEYDOWN)
      {
        ke->m_action = EventAction::KeyDown;
      }
      else
      {
        ke->m_action = EventAction::KeyUp;
      }

      ke->m_keyCode = e.key.keysym.sym;
      ke->m_mode = modState;
    }
  }

  void ClearPool()
  {
    EventPool& pool = Main::GetInstance()->m_eventPool;
    for (Event* e : pool)
    {
      SafeDel(e);
    }
    pool.clear();
  }

}