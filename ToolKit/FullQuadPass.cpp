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
#include "ToolKit.h"

namespace ToolKit
{

  FullQuadPass::FullQuadPass() : Pass("FullQuadPass")
  {
    m_camera                   = MakeNewPtr<Camera>();
    m_quad                     = MakeNewPtr<Quad>();

    m_material                 = MakeNewPtr<Material>();
    m_material->m_vertexShader = GetShaderManager()->Create<Shader>(ShaderPath("fullQuadVert.shader", true));
  }

  void FullQuadPass::Render()
  {
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.frameBuffer, m_params.clearFrameBuffer);
    renderer->SetCamera(m_camera, true);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs(jobs, m_quad);
    renderer->Render(jobs);
  }

  void FullQuadPass::PreRender()
  {
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
  }

  void FullQuadPass::PostRender()
  {
    Pass::PostRender();
    GetRenderer()->EnableDepthTest(true);
  }

  void FullQuadPass::SetFragmentShader(ShaderPtr fragmentShader, Renderer* renderer)
  {
    m_material->m_fragmentShader = fragmentShader;
    m_program = GetGpuProgramManager()->CreateProgram(m_material->m_vertexShader, m_material->m_fragmentShader);
    renderer->BindProgram(m_program);
  }

} // namespace ToolKit
