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
    camera          = MakeNewPtr<Camera>();
    frameBuffer     = MakeNewPtr<Framebuffer>();
    renderTarget    = MakeNewPtr<RenderTarget>();
    m_sceneRenderer = MakeNewPtr<SceneRenderer>();
    ScenePtr scene  = MakeNewPtr<Scene>();
    scene->Init();
    GetSceneManager()->SetCurrentScene(scene);
  }

  void Game::Destroy() {}

  void Game::Frame(float deltaTime, class Viewport* viewport)
  {
    uint width  = (uint) viewport->m_wndContentAreaSize.x;
    uint height = (uint) viewport->m_wndContentAreaSize.y;

    frameBuffer->Init({width, height});
    frameBuffer->ReconstructIfNeeded(width, height);

    camera->SetLens(glm::radians(90.0f),
                    viewport->m_wndContentAreaSize.y / viewport->m_wndContentAreaSize.x,
                    0.01f,
                    1000.0f);

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.InternalFormat      = GraphicTypes::FormatRGBA8;
    oneChannelSet.Format              = GraphicTypes::FormatRGBA;
    oneChannelSet.Type                = GraphicTypes::TypeUnsignedByte;

    renderTarget->m_settings          = oneChannelSet;
    renderTarget->ReconstructIfNeeded(width, height);
    frameBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, renderTarget);

    m_sceneRenderer->m_params.Cam                    = camera;
    m_sceneRenderer->m_params.ClearFramebuffer       = true;
    m_sceneRenderer->m_params.Gfx.BloomIntensity     = 0.0;
    m_sceneRenderer->m_params.Lights                 = GetSceneManager()->GetCurrentScene()->GetLights();
    m_sceneRenderer->m_params.MainFramebuffer        = frameBuffer;
    m_sceneRenderer->m_params.Scene                  = GetSceneManager()->GetCurrentScene();
    m_sceneRenderer->m_params.Gfx.SSAOEnabled        = false;
    m_sceneRenderer->m_params.Gfx.TonemappingEnabled = false;

    GetRenderSystem()->AddRenderTask({[&](Renderer* renderer) -> void { renderer->SetFramebuffer(frameBuffer); }});
    static uint totalFrameCount = 0;
    GetRenderSystem()->AddRenderTask(m_sceneRenderer);
    GetRenderSystem()->ExecuteRenderTasks();
    camera->m_node->SetTranslation(Vec3(0.0f, sinf(totalFrameCount * deltaTime) * 2.5f, 0.0f));
    GetRenderSystem()->SetFrameCount(totalFrameCount++);
  }
} // namespace ToolKit