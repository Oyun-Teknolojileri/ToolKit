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
#include "Object.h"
#include "Shader.h"
#include "Scene.h"
#include "Primative.h"
#include "GradientSky.h"
#include "glm/gtc/random.hpp"
#include "stdafx.h"
#include "TKOpenGL.h"

extern "C" TK_GAME_API ToolKit::Game* TK_STDCAL CreateInstance() { return new ToolKit::Game(); }

namespace ToolKit
{
  void Game::Init(Main* master)
  {
    material = MakeNewPtr<Material>();
    shader   = GetShaderManager()->Create<Shader>(ShaderPath("unlitFrag.shader", true));
    material->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("fullQuadVert.shader", true));
    material->m_fragmentShader = shader;
    material->Init();

    cubeMaterial  = MakeNewPtr<Material>();
    cubeMaterial->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("defaultVertex.shader", true));
    cubeMaterial->m_fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("defaultFragment.shader", true));
    cubeMaterial->Init();
    camera = MakeNewPtr<Camera>();

    frameBuffer   = MakeNewPtr<Framebuffer>();
    renderTarget  = MakeNewPtr<RenderTarget>();

    ScenePtr scene = MakeNewPtr<Scene>();
    const float maxX = 10.0f;
    const float maxZ = 15.0f;
    const int numMatrices = 10;

    // Create and populate random transformation matrices
    for (int i = 0; i < numMatrices; ++i) 
    {
      glm::mat4 randomMatrix(1.0f); // Initialize as an identity matrix

      // Generate random translation within the specified area
      glm::vec3 translation(
        glm::linearRand(-maxX, maxX),
        glm::linearRand(-3.0f, 3.0f),
        glm::linearRand(-maxZ, -2.0f)
      );

      // Generate random rotation angles (in radians)
      float angleX = glm::linearRand(0.0f, glm::two_pi<float>());
      float angleY = glm::linearRand(0.0f, glm::two_pi<float>());
      float angleZ = glm::linearRand(0.0f, glm::two_pi<float>());

      // Apply translation and rotation to the matrix
      randomMatrix = glm::translate(randomMatrix, translation);
      randomMatrix = glm::rotate(randomMatrix, angleX, glm::vec3(1.0f, 0.0f, 0.0f));
      randomMatrix = glm::rotate(randomMatrix, angleY, glm::vec3(0.0f, 1.0f, 0.0f));
      randomMatrix = glm::rotate(randomMatrix, angleZ, glm::vec3(0.0f, 0.0f, 1.0f));
      randomMatrix = glm::scale(randomMatrix, glm::vec3(2.0f));

      CubePtr cube = MakeNewPtr<Cube>();
      cube->m_node->SetTransform(randomMatrix);
      scene->AddEntity(cube);
    }

    scene->AddEntity(MakeNewPtr<GradientSky>());
    scene->AddEntity(MakeNewPtr<DirectionalLight>());
    scene->Init();

    GetSceneManager()->SetCurrentScene(scene);
  }

  void Game::Destroy() { }

  void Game::Frame(float deltaTime, class Viewport* viewport)
  {
    uint width  = (uint)viewport->m_wndContentAreaSize.x;
    uint height = (uint)viewport->m_wndContentAreaSize.y;

    frameBuffer->Init({width, height});
    frameBuffer->ReconstructIfNeeded(width, height);

    camera->SetLens(glm::radians(90.0f), viewport->m_wndContentAreaSize.y / viewport->m_wndContentAreaSize.x, 0.01f, 1000.0f);

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.InternalFormat      = GraphicTypes::FormatRGBA8;
    oneChannelSet.Format              = GraphicTypes::FormatRGBA;
    oneChannelSet.Type                = GraphicTypes::TypeUnsignedByte;

    renderTarget->m_settings          = oneChannelSet;
    renderTarget->ReconstructIfNeeded(width, height);
    frameBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, renderTarget);

    shader->SetShaderParameter("Color", ParameterVariant(Vec4(0.0f, 1.0f, 0.0f, 1.0f)));
    cubeMaterial->m_fragmentShader->SetShaderParameter("Color", ParameterVariant(Vec4(0.0f, 1.0f, 0.0f, 1.0f)));

    m_sceneRenderer.m_params.Cam                = camera;
    m_sceneRenderer.m_params.ClearFramebuffer   = true;
    m_sceneRenderer.m_params.Gfx.BloomIntensity = 0.0;
    m_sceneRenderer.m_params.Lights             = GetSceneManager()->GetCurrentScene()->GetLights();
    m_sceneRenderer.m_params.MainFramebuffer    = frameBuffer;
    m_sceneRenderer.m_params.Scene              = GetSceneManager()->GetCurrentScene();
    m_sceneRenderer.m_params.Gfx.SSAOEnabled    = false;
    m_sceneRenderer.m_params.Gfx.TonemappingEnabled = false;
    GetRenderSystem()->AddRenderTask({[&](Renderer* renderer)-> void
                                     {
                                     renderer->SetFramebuffer(frameBuffer);
                                     }});
    static uint totalFrameCount = 0;
    GetRenderSystem()->AddRenderTask(&m_sceneRenderer);
    GetRenderSystem()->ExecuteRenderTasks();
    camera->m_node->SetTranslation(Vec3(0.0f, sinf(totalFrameCount * deltaTime) * 2.5f, 0.0f));
    GetRenderSystem()->SetFrameCount(totalFrameCount++);
  }
} // namespace ToolKit