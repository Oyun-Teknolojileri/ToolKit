/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "CubemapPass.h"

#include "Material.h"
#include "Mesh.h"
#include "TKProfiler.h"
#include "ToolKit.h"

namespace ToolKit
{

  CubeMapPass::CubeMapPass() { m_cube = MakeNewPtr<Cube>(); }

  CubeMapPass::CubeMapPass(const CubeMapPassParams& params) : CubeMapPass() { m_params = params; }

  CubeMapPass::~CubeMapPass() { m_cube = nullptr; }

  void CubeMapPass::Render()
  {
    PUSH_GPU_MARKER("CubeMapPass::Render");
    PUSH_CPU_MARKER("CubeMapPass::Render");

    m_cube->m_node->SetTransform(m_params.Transform);

    Renderer* renderer = GetRenderer();

    if (m_params.ClearFramebuffer)
    {
      renderer->SetFramebuffer(m_params.FrameBuffer, GraphicBitFields::AllBits);
    }
    else
    {
      renderer->SetFramebuffer(m_params.FrameBuffer, GraphicBitFields::None);
    }

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs({m_cube}, jobs);
    renderer->RenderWithProgramFromMaterial(jobs);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void CubeMapPass::PreRender()
  {
    PUSH_GPU_MARKER("CubeMapPass::PreRender");
    PUSH_CPU_MARKER("CubeMapPass::PreRender");

    Pass::PreRender();
    MaterialComponentPtr matCom = m_cube->GetMaterialComponent();
    matCom->SetFirstMaterial(m_params.Material);

    Renderer* renderer = GetRenderer();
    renderer->SetDepthTestFunc(m_params.DepthFn);
    renderer->SetCamera(m_params.Cam, false);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void CubeMapPass::PostRender()
  {
    PUSH_GPU_MARKER("CubeMapPass::PostRender");
    PUSH_CPU_MARKER("CubeMapPass::PostRender");

    Pass::PostRender();
    GetRenderer()->SetDepthTestFunc(CompareFunctions::FuncLess);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit