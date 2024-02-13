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

    if (m_params.ClearFrameBuffer)
    {
      renderer->SetFramebuffer(m_params.FrameBuffer, GraphicBitFields::AllBits);
    }
    else
    {
      renderer->SetFramebuffer(m_params.FrameBuffer, GraphicBitFields::None);
    }

    renderer->SetCamera(m_camera, true);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs({m_quad}, jobs);
    renderer->Render(jobs);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void FullQuadPass::PreRender()
  {
    PUSH_GPU_MARKER("FullQuadPass::PreRender");
    PUSH_CPU_MARKER("FullQuadPass::PreRender");

    Pass::PreRender();
    Renderer* renderer = GetRenderer();
    renderer->EnableDepthTest(false);

    m_material->m_fragmentShader = m_params.FragmentShader;
    m_material->UnInit(); // Reinit in case, shader change.
    m_material->Init();
    m_material->GetRenderState()->blendFunction = m_params.BlendFunc;

    m_program = GetGpuProgramManager()->CreateProgram(m_material->m_vertexShader, m_material->m_fragmentShader);

    for (int i = 0; i < m_params.shaderUniforms.size(); ++i)
    {
      m_program->UpdateCustomUniform(m_params.shaderUniforms[i]);
    }

    renderer->BindProgram(m_program);

    MeshComponentPtr mc = m_quad->GetMeshComponent();
    MeshPtr mesh        = mc->GetMeshVal();
    mesh->m_material    = m_material;
    mesh->Init();

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

} // namespace ToolKit
