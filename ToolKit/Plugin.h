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

#ifdef _WIN32 // Windows.
  #define TK_GAME_API __declspec(dllexport)
#else // Other OS.
  #define TK_GAME_API
#endif

namespace ToolKit
{

  enum class PluginType
  {
    Base,
    Game
  };

  class TK_API Plugin
  {
   public:
    Plugin() {}

    virtual ~Plugin() {}

    virtual PluginType GetType()          = 0;
    virtual void Init(class Main* master) = 0;
    virtual void Destroy()                = 0;
  };

  class TK_API GamePlugin : public Plugin
  {
   public:
    virtual void Frame(float deltaTime, class Viewport* viewport) = 0;

    PluginType GetType() { return PluginType::Game; }

   public:
    bool m_quit = false; // Set this flag true to stop gameplay.
  };

} // namespace ToolKit
