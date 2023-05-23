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

#include "Plugin.h"
#include "Types.h"

#include <functional>
#include <vector>

namespace ToolKit
{

  struct PluginRegister
  {
    Plugin* m_plugin;
    void* m_module;
    String m_lastWriteTime;
    String m_file;
    bool m_loaded;
  };

  class TK_API PluginManager
  {
   public:
    PluginManager();
    ~PluginManager();

    // Platform dependent functions.
    bool Load(const String& file); // Auto reloads if the dll is dirty.
    void Unload(const String& file);

    // No platform dependency.
    void Init();
    void UnInit();
    PluginRegister* GetRegister(const String& file);
    void Report(const char* msg, ...);

    // Shorts for game plugin.
    GamePlugin* GetGamePlugin();
    void UnloadGamePlugin();

   private:
    PluginRegister* GetGameRegister();

   public:
    std::vector<PluginRegister> m_storage;
    std::function<void(const String&)> m_reporterFn;
  };

} // namespace ToolKit
