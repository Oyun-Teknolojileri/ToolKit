/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Events.h"
#include "SDL.h"
#include "ToolKit.h"

namespace ToolKit
{
  class SDLEventPool
  {
   public:
    SDLEventPool()
    {
      for (uint64 i = 0; i < m_mouseEventPoolSize; ++i)
      {
        m_mouseEventPool.push_back(new MouseEvent());
      }
      for (uint64 i = 0; i < m_keyboardEventPoolSize; ++i)
      {
        m_keyboardEventPool.push_back(new KeyboardEvent());
      }
      for (uint64 i = 0; i < m_gamepadEventPoolSize; ++i)
      {
        m_gamepadEventPool.push_back(new GamepadEvent());
      }
      for (uint64 i = 0; i < m_touchEventPoolSize; ++i)
      {
        m_touchEventPool.push_back(new TouchEvent());
      }
    }

    ~SDLEventPool()
    {
      for (MouseEvent* me : m_mouseEventPool)
      {
        delete me;
      }
      m_mouseEventPool.clear();
      for (KeyboardEvent* ke : m_keyboardEventPool)
      {
        delete ke;
      }
      m_keyboardEventPool.clear();
      for (GamepadEvent* gpe : m_gamepadEventPool)
      {
        delete gpe;
      }
      m_gamepadEventPool.clear();
      for (TouchEvent* te : m_touchEventPool)
      {
        delete te;
      }
      m_touchEventPool.clear();
    }

    // Utility functions for Pooling & Releaseing SDL events for ToolKit.
    inline void PoolEvent(const SDL_Event& e)
    {
      if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
      {
        if (m_mouseEventPoolCurrentIndex >= m_mouseEventPoolSize)
        {
          return;
        }

        MouseEvent* me = m_mouseEventPool[m_mouseEventPoolCurrentIndex++];
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
      else if (e.type == SDL_FINGERDOWN || e.type == SDL_FINGERUP)
      {
        if (m_touchEventPoolCurrentIndex >= m_touchEventPoolSize)
        {
          return;
        }

        TouchEvent* te = m_touchEventPool[m_touchEventPoolCurrentIndex++];
        Main::GetInstance()->m_eventPool.push_back(te);

        if (e.type == SDL_FINGERDOWN)
        {
          te->m_release = false;
        }
        else
        {
          te->m_release = true;
        }

        te->m_action = EventAction::Touch;
      }
      else if (e.type == SDL_FINGERMOTION)
      {
        if (m_touchEventPoolCurrentIndex >= m_touchEventPoolSize)
        {
          return;
        }

        TouchEvent* te = m_touchEventPool[m_touchEventPoolCurrentIndex++];
        Main::GetInstance()->m_eventPool.push_back(te);

        te->m_action    = EventAction::Move;
        te->absolute[0] = e.tfinger.x;
        te->absolute[1] = e.tfinger.y;
      }
      else if (e.type == SDL_MOUSEMOTION)
      {
        if (m_mouseEventPoolCurrentIndex >= m_mouseEventPoolSize)
        {
          return;
        }

        MouseEvent* me = m_mouseEventPool[m_mouseEventPoolCurrentIndex++];
        Main::GetInstance()->m_eventPool.push_back(me);

        me->m_action    = EventAction::Move;
        me->absolute[0] = e.motion.x;
        me->absolute[1] = e.motion.y;
        me->relative[0] = e.motion.xrel;
        me->relative[1] = e.motion.yrel;
      }
      else if (e.type == SDL_MOUSEWHEEL)
      {
        if (m_mouseEventPoolCurrentIndex >= m_mouseEventPoolSize)
        {
          return;
        }

        MouseEvent* me = m_mouseEventPool[m_mouseEventPoolCurrentIndex++];
        Main::GetInstance()->m_eventPool.push_back(me);

        me->m_action  = EventAction::Scroll;
        me->scroll[0] = e.wheel.x;
        me->scroll[1] = e.wheel.y;
      }
      else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
      {
        if (m_keyboardEventPoolCurrentIndex >= m_keyboardEventPoolSize)
        {
          return;
        }

        SDL_Keymod modState = SDL_GetModState();

        KeyboardEvent* ke   = m_keyboardEventPool[m_keyboardEventPoolCurrentIndex++];
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
        ke->m_mode    = modState;
      }

      if (m_gamepadEventPoolCurrentIndex >= m_gamepadEventPoolSize)
      {
        return;
      }

      GamepadEvent* ge = nullptr;

      switch (e.type)
      {
      case SDL_CONTROLLERAXISMOTION:
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
        ge = m_gamepadEventPool[m_gamepadEventPoolCurrentIndex++];
      };

      // handle joystick events
      switch (e.type)
      {
      case SDL_CONTROLLERAXISMOTION:
        ge->m_action = EventAction::GamepadAxis;
        ge->m_angle  = e.caxis.value / (float) (SDL_JOYSTICK_AXIS_MAX);
        ge->m_axis   = (GamepadEvent::StickAxis) e.caxis.axis;
        break;
      case SDL_CONTROLLERBUTTONDOWN:
        ge->m_action = EventAction::GamepadButtonDown;
        ge->m_button = (GamepadButton) (1 << e.cbutton.button);
        break;
      case SDL_CONTROLLERBUTTONUP:
        ge->m_action = EventAction::GamepadButtonUp;
        ge->m_button = (GamepadButton) (1 << e.cbutton.button);
        break;
      case SDL_CONTROLLERDEVICEADDED:
        GetLogger()->WriteConsole(LogType::Memo, "Gamepad connected!");
        SDL_GameControllerOpen(e.cdevice.which);
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        GetLogger()->WriteConsole(LogType::Memo, "Gamepad disconnected!");
        break;
      };

      if (ge != nullptr)
      {
        Main::GetInstance()->m_eventPool.push_back(ge);
      }
    } // void PoolEvent end

    inline void ClearPool()
    {
      EventPool& pool = Main::GetInstance()->m_eventPool;
      pool.clear();

      m_mouseEventPoolCurrentIndex    = 0;
      m_keyboardEventPoolCurrentIndex = 0;
      m_gamepadEventPoolCurrentIndex  = 0;
    }

   private:
    const uint64 m_mouseEventPoolSize      = 1024;
    const uint64 m_keyboardEventPoolSize   = 1024;
    const uint64 m_gamepadEventPoolSize    = 1024;
    const uint64 m_touchEventPoolSize      = 1024;
    uint64 m_mouseEventPoolCurrentIndex    = 0;
    uint64 m_keyboardEventPoolCurrentIndex = 0;
    uint64 m_gamepadEventPoolCurrentIndex  = 0;
    uint64 m_touchEventPoolCurrentIndex  = 0;
    std::vector<MouseEvent*> m_mouseEventPool;
    std::vector<KeyboardEvent*> m_keyboardEventPool;
    std::vector<GamepadEvent*> m_gamepadEventPool;
    std::vector<TouchEvent*> m_touchEventPool;
  };
} // namespace ToolKit