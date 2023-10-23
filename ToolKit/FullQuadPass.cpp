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

#include "FullQuadPass.h"

#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  FullQuadPass::FullQuadPass()
  {
    m_camera                   = MakeNewPtr<Camera>(); // Unused.
    m_quad                     = MakeNewPtr<Quad>();

    m_material                 = MakeNewPtr<Material>();
    m_material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("fullQuadVert.shader", true));
  }

  FullQuadPass::FullQuadPass(const FullQuadPassParams& params) : FullQuadPass() { m_params = params; }

  FullQuadPass::~FullQuadPass()
  {
    m_camera   = nullptr;
    m_quad     = nullptr;
    m_material = nullptr;
  }

  void FullQuadPass::Render()
  {
    PUSH_GPU_MARKER("FullQuadPass::Render");

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.ClearFrameBuffer, {0.0f, 0.0f, 0.0f, 1.0f});

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs({m_quad}, jobs);
    renderer->Render(jobs, m_camera, {});

    POP_GPU_MARKER();
  }

  void FullQuadPass::PreRender()
  {
    PUSH_GPU_MARKER("FullQuadPass::PreRender");

    Pass::PreRender();
    Renderer* renderer      = GetRenderer();
    renderer->m_overrideMat = nullptr;
    renderer->EnableDepthTest(false);

    m_material->m_fragmentShader = m_params.FragmentShader;
    m_material->UnInit(); // Reinit in case, shader change.
    m_material->Init();
    m_material->GetRenderState()->blendFunction = m_params.BlendFunc;

    MeshComponentPtr mc                         = m_quad->GetMeshComponent();
    MeshPtr mesh                                = mc->GetMeshVal();
    mesh->m_material                            = m_material;
    mesh->Init();

    POP_GPU_MARKER();
  }

  void FullQuadPass::PostRender()
  {
    PUSH_GPU_MARKER("FullQuadPass::PostRender");

    Pass::PostRender();
    GetRenderer()->EnableDepthTest(true);

    POP_GPU_MARKER();
  }

} // namespace ToolKit