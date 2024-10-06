/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "CubemapPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"

namespace ToolKit
{

  CubeMapPass::CubeMapPass() { m_cube = MakeNewPtr<Cube>(); }

  CubeMapPass::CubeMapPass(const CubeMapPassParams& params) : CubeMapPass() { m_params = params; }

  CubeMapPass::~CubeMapPass() { m_cube = nullptr; }

  void CubeMapPass::Render()
  {
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_params.FrameBuffer, m_params.clearBuffer);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs(jobs, m_cube);

    renderer->RenderWithProgramFromMaterial(jobs);
  }

  void CubeMapPass::PreRender()
  {
    Pass::PreRender();

    m_cube->m_node->SetTransform(m_params.Transform);

    MaterialComponentPtr matCom = m_cube->GetMaterialComponent();
    matCom->SetFirstMaterial(m_params.Material);

    Renderer* renderer = GetRenderer();
    renderer->SetDepthTestFunc(m_params.DepthFn);
    renderer->SetCamera(m_params.Cam, false);
  }

  void CubeMapPass::PostRender()
  {
    Pass::PostRender();
    GetRenderer()->SetDepthTestFunc(CompareFunctions::FuncLess);
  }

} // namespace ToolKit