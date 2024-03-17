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
#include "Logger.h"

namespace ToolKit
{

  template <PLATFORM Platform>
  class SDLEventPool
  {
   public:
    SDLEventPool()
    {
      for (uint16 i = 0; i < m_mouseEventPoolSize; ++i)
      {
        m_mouseEventPool.push_back(new MouseEvent());
      }
      for (uint16 i = 0; i < m_keyboardEventPoolSize; ++i)
      {
        m_keyboardEventPool.push_back(new KeyboardEvent());
      }
      for (uint16 i = 0; i < m_gamepadEventPoolSize; ++i)
      {
        m_gamepadEventPool.push_back(new GamepadEvent());
      }
      for (uint16 i = 0; i < m_touchEventPoolSize; ++i)
      {
        m_touchEventPool.push_back(new TouchEvent());
      }
    }

    ~SDLEventPool()
    {
      for (MouseEvent* mouseEvent : m_mouseEventPool)
      {
        SafeDel(mouseEvent);
      }
      m_mouseEventPool.clear();
      for (KeyboardEvent* keyboardEvent : m_keyboardEventPool)
      {
        SafeDel(keyboardEvent);
      }
      m_keyboardEventPool.clear();
      for (GamepadEvent* gamepadEvent : m_gamepadEventPool)
      {
        SafeDel(gamepadEvent);
      }
      m_gamepadEventPool.clear();
      for (TouchEvent* touchEvent : m_touchEventPool)
      {
        SafeDel(touchEvent);
      }
      m_touchEventPool.clear();
    }

    // Utility functions for Pooling & Releaseing SDL events for ToolKit.
    inline void PoolEvent(const SDL_Event& event)
    {
      if constexpr (Platform != PLATFORM::TKAndroid)
      {
        if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
        {
          if (m_mouseEventPoolCurrentIndex >= m_mouseEventPoolSize)
          {
            return;
          }

          MouseEvent* mouseEvent = m_mouseEventPool[m_mouseEventPoolCurrentIndex++];
          Main::GetInstance()->m_eventPool.push_back(mouseEvent);

          if (event.type == SDL_MOUSEBUTTONDOWN)
          {
            mouseEvent->m_release = false;
          }
          else
          {
            mouseEvent->m_release = true;
          }

          switch (event.button.button)
          {
          case SDL_BUTTON_LEFT:
            mouseEvent->m_action = EventAction::LeftClick;
            break;
          case SDL_BUTTON_RIGHT:
            mouseEvent->m_action = EventAction::RightClick;
            break;
          case SDL_BUTTON_MIDDLE:
            mouseEvent->m_action = EventAction::MiddleClick;
            break;
          }
          mouseEvent->absolute[0] = event.motion.x;
          mouseEvent->absolute[1] = event.motion.y;
          mouseEvent->relative[0] = event.motion.xrel;
          mouseEvent->relative[1] = event.motion.yrel;
        }
        else if (event.type == SDL_MOUSEMOTION)
        {
          if (m_mouseEventPoolCurrentIndex >= m_mouseEventPoolSize)
          {
            return;
          }

          MouseEvent* mouseEvent = m_mouseEventPool[m_mouseEventPoolCurrentIndex++];
          Main::GetInstance()->m_eventPool.push_back(mouseEvent);

          mouseEvent->m_action    = EventAction::Move;
          mouseEvent->absolute[0] = event.motion.x;
          mouseEvent->absolute[1] = event.motion.y;
          mouseEvent->relative[0] = event.motion.xrel;
          mouseEvent->relative[1] = event.motion.yrel;
        }
        else if (event.type == SDL_MOUSEWHEEL)
        {
          if (m_mouseEventPoolCurrentIndex >= m_mouseEventPoolSize)
          {
            return;
          }

          MouseEvent* mouseEvent = m_mouseEventPool[m_mouseEventPoolCurrentIndex++];
          Main::GetInstance()->m_eventPool.push_back(mouseEvent);

          mouseEvent->m_action  = EventAction::Scroll;
          mouseEvent->scroll[0] = event.wheel.x;
          mouseEvent->scroll[1] = event.wheel.y;
        }
      }
      else
      {
          if (event.type == SDL_FINGERDOWN || event.type == SDL_FINGERUP)
          {
            if (m_touchEventPoolCurrentIndex >= m_touchEventPoolSize)
            {
              return;
            }

            TouchEvent* touchEvent = m_touchEventPool[m_touchEventPoolCurrentIndex++];
            Main::GetInstance()->m_eventPool.push_back(touchEvent);

            if (event.type == SDL_FINGERDOWN)
            {
              touchEvent->m_release = false;
            }
            else
            {
              touchEvent->m_release = true;
            }

            touchEvent->m_action    = EventAction::Touch;
            touchEvent->absolute[0] = event.tfinger.x;
            touchEvent->absolute[1] = event.tfinger.y;
          }
          else if (event.type == SDL_FINGERMOTION)
          {
            if (m_touchEventPoolCurrentIndex >= m_touchEventPoolSize)
            {
              return;
            }

            TouchEvent* touchEvent = m_touchEventPool[m_touchEventPoolCurrentIndex++];
            Main::GetInstance()->m_eventPool.push_back(touchEvent);

            touchEvent->m_action    = EventAction::Move;
            touchEvent->absolute[0] = event.tfinger.x;
            touchEvent->absolute[1] = event.tfinger.y;
          }
      }

      if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      {
        if (m_keyboardEventPoolCurrentIndex >= m_keyboardEventPoolSize)
        {
          return;
        }

        SDL_Keymod modState          = SDL_GetModState();

        KeyboardEvent* keyboardEvent = m_keyboardEventPool[m_keyboardEventPoolCurrentIndex++];
        Main::GetInstance()->m_eventPool.push_back(keyboardEvent);

        if (event.type == SDL_KEYDOWN)
        {
          keyboardEvent->m_action = EventAction::KeyDown;
        }
        else
        {
          keyboardEvent->m_action = EventAction::KeyUp;
        }

        keyboardEvent->m_keyCode = event.key.keysym.sym;
        keyboardEvent->m_mode    = modState;
      }

      if (m_gamepadEventPoolCurrentIndex >= m_gamepadEventPoolSize)
      {
        return;
      }

      GamepadEvent* gamepadEvent = nullptr;

      switch (event.type)
      {
      case SDL_CONTROLLERAXISMOTION:
      case SDL_CONTROLLERBUTTONDOWN:
      case SDL_CONTROLLERBUTTONUP:
        gamepadEvent = m_gamepadEventPool[m_gamepadEventPoolCurrentIndex++];
      };

      // handle joystick events
      switch (event.type)
      {
      case SDL_CONTROLLERAXISMOTION:
        gamepadEvent->m_action = EventAction::GamepadAxis;
        gamepadEvent->m_angle  = event.caxis.value / (float) (SDL_JOYSTICK_AXIS_MAX);
        gamepadEvent->m_axis   = (GamepadEvent::StickAxis) event.caxis.axis;
        break;
      case SDL_CONTROLLERBUTTONDOWN:
        gamepadEvent->m_action = EventAction::GamepadButtonDown;
        gamepadEvent->m_button = (GamepadButton) (1 << event.cbutton.button);
        break;
      case SDL_CONTROLLERBUTTONUP:
        gamepadEvent->m_action = EventAction::GamepadButtonUp;
        gamepadEvent->m_button = (GamepadButton) (1 << event.cbutton.button);
        break;
      case SDL_CONTROLLERDEVICEADDED:
        TK_LOG("Gamepad connected.");
        SDL_GameControllerOpen(event.cdevice.which);
        break;
      case SDL_CONTROLLERDEVICEREMOVED:
        TK_LOG("Gamepad disconnected.");
        break;
      };

      if (gamepadEvent != nullptr)
      {
        Main::GetInstance()->m_eventPool.push_back(gamepadEvent);
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
    const uint16 m_mouseEventPoolSize      = 1024;
    const uint16 m_keyboardEventPoolSize   = 1024;
    const uint16 m_gamepadEventPoolSize    = 1024;
    const uint16 m_touchEventPoolSize      = 1024;
    uint16 m_mouseEventPoolCurrentIndex    = 0;
    uint16 m_keyboardEventPoolCurrentIndex = 0;
    uint16 m_gamepadEventPoolCurrentIndex  = 0;
    uint16 m_touchEventPoolCurrentIndex    = 0;
    std::vector<MouseEvent*> m_mouseEventPool;
    std::vector<KeyboardEvent*> m_keyboardEventPool;
    std::vector<GamepadEvent*> m_gamepadEventPool;
    std::vector<TouchEvent*> m_touchEventPool;
  };

} // namespace ToolKit