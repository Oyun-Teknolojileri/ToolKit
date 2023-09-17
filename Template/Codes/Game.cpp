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

#include "Game.h"

extern "C" TK_GAME_API ToolKit::Game* TK_STDCAL CreateInstance() { return new ToolKit::Game(); }

namespace ToolKit
{

  void Game::Init(Main* master) { Main::SetProxy(master); }

  void Game::Destroy() { delete this; }

  void Game::Frame(float deltaTime, class Viewport* viewport)
  {
#if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
    GetRenderSystem()->ExecuteRenderTasks();

    m_sceneRenderer.m_params.Cam                = camera;
    m_sceneRenderer.m_params.ClearFramebuffer   = true;
    m_sceneRenderer.m_params.Gfx.BloomIntensity = 0.0;
    m_sceneRenderer.m_params.Lights             = GetSceneManager()->GetCurrentScene()->GetLights();
    m_sceneRenderer.m_params.MainFramebuffer    = viewport->m_framebuffer;
    m_sceneRenderer.m_params.Scene              = GetSceneManager()->GetCurrentScene();
    GetRenderSystem()->AddRenderTask(&m_sceneRenderer);

    static uint totalFrameCount = 0;
    GetRenderSystem()->SetFrameCount(totalFrameCount++);
#endif
  }

} // namespace ToolKit