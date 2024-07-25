/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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
    m_camera                   = MakeNewPtr<Camera>();
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
    PUSH_CPU_MARKER("FullQuadPass::Render");

    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.frameBuffer, m_params.clearFrameBuffer);
    renderer->SetCamera(m_camera, true);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs(jobs, {m_quad.get()});
    renderer->Render(jobs);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void FullQuadPass::PreRender()
  {
    PUSH_GPU_MARKER("FullQuadPass::PreRender");
    PUSH_CPU_MARKER("FullQuadPass::PreRender");

    // Gpu Program should be bound before calling FulQuadPass Render

    Pass::PreRender();
    Renderer* renderer = GetRenderer();
    renderer->EnableDepthTest(false);

    MeshComponentPtr mc = m_quad->GetMeshComponent();
    MeshPtr mesh        = mc->GetMeshVal();
    mesh->m_material    = m_material;
    mesh->Init();

    m_material->GetRenderState()->blendFunction = m_params.blendFunc;
    SetFragmentShader(m_material->m_fragmentShader, renderer);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void FullQuadPass::PostRender()
  {
    PUSH_GPU_MARKER("FullQuadPass::PostRender");
    PUSH_CPU_MARKER("FullQuadPass::PostRender");

    Pass::PostRender();
    GetRenderer()->EnableDepthTest(true);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void FullQuadPass::SetFragmentShader(ShaderPtr fragmentShader, Renderer* renderer)
  {
    m_material->m_fragmentShader = fragmentShader;
    m_program = GetGpuProgramManager()->CreateProgram(m_material->m_vertexShader, m_material->m_fragmentShader);
    renderer->BindProgram(m_program);
  }

} // namespace ToolKit
