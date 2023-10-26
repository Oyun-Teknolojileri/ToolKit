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
      for (size_t i = 0; i < m_mouseEventPoolSize; ++i)
      {
        m_mouseEventPool.push_back(new MouseEvent());
      }
      for (size_t i = 0; i < m_keyboardEventPoolSize; ++i)
      {
        m_keyboardEventPool.push_back(new KeyboardEvent());
      }
      for (size_t i = 0; i < m_gamepadEventPoolSize; ++i)
      {
        m_gamepadEventPool.push_back(new GamepadEvent());
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

      if (e.type == SDL_MOUSEMOTION)
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

      if (e.type == SDL_MOUSEWHEEL)
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

      if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
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
    const size_t m_mouseEventPoolSize      = 1024;
    const size_t m_keyboardEventPoolSize   = 1024;
    const size_t m_gamepadEventPoolSize    = 1024;
    size_t m_mouseEventPoolCurrentIndex    = 0;
    size_t m_keyboardEventPoolCurrentIndex = 0;
    size_t m_gamepadEventPoolCurrentIndex  = 0;
    std::vector<MouseEvent*> m_mouseEventPool;
    std::vector<KeyboardEvent*> m_keyboardEventPool;
    std::vector<GamepadEvent*> m_gamepadEventPool;
  };
} // namespace ToolKit