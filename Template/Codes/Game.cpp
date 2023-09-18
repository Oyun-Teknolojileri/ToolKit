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

#include "GradientSky.h"
#include "Object.h"
#include "Primative.h"
#include "Scene.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "glm/gtc/random.hpp"
#include "stdafx.h"

extern "C" TK_GAME_API ToolKit::Game* TK_STDCAL CreateInstance() { return new ToolKit::Game(); }

namespace ToolKit
{
  void Game::Init(Main* master)
  { 
    m_sceneRenderer = MakeNewPtr<SceneRenderer>();
    ScenePtr scene  = GetSceneManager()->Create<Scene>(ScenePath("Engine/Scenes/ms-ball.scene"));
    m_sceneRenderer->m_params.Scene = scene;
  }

  void Game::Destroy() { delete this; }

  void Game::Frame(float deltaTime, class Viewport* viewport)
  {
    m_sceneRenderer->m_params.MainFramebuffer = viewport->m_framebuffer;
    m_sceneRenderer->m_params.Cam             = viewport->GetCamera();

    GetRenderSystem()->AddRenderTask(
        {[&](Renderer* renderer) -> void { renderer->SetFramebuffer(viewport->m_framebuffer); }});

    GetRenderSystem()->AddRenderTask(m_sceneRenderer);
    GetRenderSystem()->ExecuteRenderTasks();
    static int totalFrameCount = 0;
    GetRenderSystem()->SetFrameCount(totalFrameCount++);
  }
} // namespace ToolKit