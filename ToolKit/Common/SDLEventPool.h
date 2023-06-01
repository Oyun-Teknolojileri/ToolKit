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

#include "SDL.h"
#include "ToolKit.h"

namespace ToolKit
{
  static SDL_GameController* gamepad = nullptr;

  // Utility functions for Pooling & Releaseing SDL events for ToolKit.
  static void PoolEvent(const SDL_Event& e)
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

      me->m_action    = EventAction::Move;
      me->absolute[0] = e.motion.x;
      me->absolute[1] = e.motion.y;
      me->relative[0] = e.motion.xrel;
      me->relative[1] = e.motion.yrel;
    }

    if (e.type == SDL_MOUSEWHEEL)
    {
      MouseEvent* me = new MouseEvent();
      Main::GetInstance()->m_eventPool.push_back(me);

      me->m_action  = EventAction::Scroll;
      me->scroll[0] = e.wheel.x;
      me->scroll[1] = e.wheel.y;
    }

    if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP)
    {
      SDL_Keymod modState = SDL_GetModState();

      KeyboardEvent* ke   = new KeyboardEvent();
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

    // handle joystick events
    switch (e.type)
    {
      case SDL_JOYDEVICEADDED:
        gamepad = SDL_GameControllerOpen(0);
        if (gamepad != nullptr)
        {
          GetLogger()->WriteConsole(LogType::Memo, "Gamepad connected!");
        }
        break;
      case SDL_JOYDEVICEREMOVED:
        if (gamepad != nullptr)
        {
          GetLogger()->WriteConsole(LogType::Memo, "Gamepad disconnected!");
          SDL_GameControllerOpen(0);
          gamepad = nullptr;
        }
        break;
      case SDL_QUIT:
        if (gamepad != nullptr)
        {
          SDL_GameControllerClose(gamepad);
          gamepad = nullptr;
        }
        break;
    };

  } // void PoolEvent end

  static void InitializeGamepad()
  {
    // Check for available game controllers
    if (SDL_NumJoysticks() < 1)
    {
      // No game controllers connected
      return;
    }
  
    // Open the first available game controller
    gamepad = SDL_GameControllerOpen(0);
    if (gamepad == nullptr)
    {
      GetLogger()->WriteConsole(LogType::Memo,
                                "Could not open game controller! SDL_Error: %s\n",
                                SDL_GetError());
      return;
    }
  
    GetLogger()->WriteConsole(LogType::Memo, "Game controller initialized successfully!\n");
  }

  static Vec2 tk_LeftStickAxis {}, tk_RightStickAxis {};
  static uint tk_GamepadDownButtons = 0u;
  static uint tk_GamepadReleasedButtons = 0u;
  static uint tk_GamepadPressedButtons  = 0u;

  static void HandleGamepad()
  {
    if (gamepad == nullptr)
    {
      return;
    }

    uint lastButtons = tk_GamepadDownButtons;
    tk_GamepadDownButtons = 0u;

    for (uint i = 0; i < (int) GamepadButton::Count; i++)
    {
      uint isDown            = (uint)SDL_GameControllerGetButton(gamepad, (SDL_GameControllerButton)i);
      tk_GamepadDownButtons |= (1u << i) * isDown;
    }

    short rightTrigger  = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    short leftTrigger   = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_TRIGGERLEFT);

    // left trigger and right trigger can be used as both button and axis so we are setting it here
    // TrailingZeroCnt(TriggerRight) and TrailingZeroCnt(TriggerLeft) can be used instead of SDL_CONTROLLER_BUTTON_MAX
    // or you can even use 21 and 22 to indicate indexes of trigger button bits
    tk_GamepadDownButtons    |= (1u << (SDL_CONTROLLER_BUTTON_MAX + 0)) * (uint)(rightTrigger > 0);
    tk_GamepadDownButtons    |= (1u << (SDL_CONTROLLER_BUTTON_MAX + 1)) * (uint) (leftTrigger > 0);

    tk_GamepadPressedButtons  = ~lastButtons &  tk_GamepadDownButtons; // it was up and pressed 
    tk_GamepadReleasedButtons =  lastButtons & ~tk_GamepadDownButtons; // it was down and released

    // is something changed?
    if (uint64_t(tk_GamepadDownButtons + tk_GamepadPressedButtons) + tk_GamepadReleasedButtons != 0ull)
    {
      GamepadEvent* ge      = new GamepadEvent();
      ge->m_downButtons     = tk_GamepadDownButtons;
      ge->m_pressedButtons  = tk_GamepadPressedButtons;
      ge->m_releasedButtons = tk_GamepadReleasedButtons;
      ge->m_action          = EventAction::GamepadButton;
      // send event
      Main::GetInstance()->m_eventPool.push_back(ge);
    }

    const float axisMax    = float(SDL_JOYSTICK_AXIS_MAX);
    tk_LeftStickAxis.x     = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTX) / axisMax;
    tk_LeftStickAxis.y     = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_LEFTY) / axisMax;

    tk_RightStickAxis.x    = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTX) / axisMax;
    tk_RightStickAxis.y    = SDL_GameControllerGetAxis(gamepad, SDL_CONTROLLER_AXIS_RIGHTY) / axisMax;
    
    // invert y
    tk_LeftStickAxis.y     = -1.0f * tk_LeftStickAxis.y;
    tk_RightStickAxis.y    = -1.0f * tk_RightStickAxis.y;

    // no sqrt involved so no crash
    if (glm::length2(tk_LeftStickAxis) > 0.02f || glm::length2(tk_RightStickAxis) > 0.02f)
    {
      GamepadEvent* ge   = new GamepadEvent();
      ge->m_leftStickAxis  = tk_LeftStickAxis;
      ge->m_rightStickAxis = tk_RightStickAxis;
      ge->m_action         = EventAction::GamepadAxis;
      Main::GetInstance()->m_eventPool.push_back(ge);
    }
    
    if (tk_GamepadReleasedButtons & (uint)GamepadButton::Guide)
    {
      static const char* message = "Guide/Mode button pressed, d-pad and right stick might not work correctly";
      GetLogger()->WriteConsole(LogType::Memo, message);
    }
    // todo: call gamepad Test function here. if console command LogGamepadInput is set to 1
  }

  // we can use this function to test gamepads
  // warning: call this function after HandleGamepad function.
  static void GamepadTest()
  {
#ifdef _DEBUG
    static const char* buttonNames[(int)GamepadButton::Count] = {
                                                         "Xbox: A PS: ()",
                                                         "Xbox: B PS: X ",
                                                         "Xbox: X PS: /\\",
                                                         "Xbox: Y PS: []",
                                                         "Back          ",
                                                         "Guide         ",
                                                         "Start         ",
                                                         "LeftStick     ",
                                                         "RightStick    ",
                                                         "LeftShoulder  ",
                                                         "RightShoulder ",
                                                         "DpadUp        ",
                                                         "DpadDown      ",
                                                         "DpadLeft      ",
                                                         "DpadRight     ",
                                                         "Misc1         ",
                                                         "Paddle1       ",
                                                         "Paddle2       ",
                                                         "Paddle3       ",
                                                         "Paddle4       ",
                                                         "Touchpad      ",
                                                         "TriggerRight  ",
                                                         "TriggerLeft   "
    };

    for (int i = 0; i < (int)GamepadButton::Count; i++)
    {
      if (tk_GamepadPressedButtons & (1 << i))
      {
        GetLogger()->WriteConsole(LogType::Memo, "pressed  %s", buttonNames[i]);
      }
      else if (tk_GamepadDownButtons & (1 << i))
      {
        GetLogger()->WriteConsole(LogType::Memo, "down      %s", buttonNames[i]);
      }
      
      if (tk_GamepadReleasedButtons & (1 << i))
      {
        GetLogger()->WriteConsole(LogType::Memo, "released %s", buttonNames[i]);
      }
    }

    if (glm::length2(tk_LeftStickAxis) > 0.05f)
    {
      GetLogger()->WriteConsole(LogType::Memo,
                                "left stick axis: x = %f, y = %f",
                                tk_LeftStickAxis.x,
                                tk_LeftStickAxis.y);
    }
        
    if (glm::length2(tk_RightStickAxis) > 0.05f)
    {
      GetLogger()->WriteConsole(LogType::Memo,
                                "right stick axis: x = %f, y = %f",
                                tk_RightStickAxis.x,
                                tk_RightStickAxis.y);
    }
#endif
  }

  static void ClearPool()
  {
    EventPool& pool = Main::GetInstance()->m_eventPool;
    for (Event* e : pool)
    {
      SafeDel(e);
    }
    pool.clear();
  }

} // namespace ToolKit