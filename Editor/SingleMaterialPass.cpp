/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "SingleMaterialPass.h"

#include "Material.h"
#include "TKProfiler.h"



namespace ToolKit
{
  namespace Editor
  {

    SingleMatForwardRenderPass::SingleMatForwardRenderPass() : ForwardRenderPass()
    {
      m_overrideMat = MakeNewPtr<Material>();
    }

    SingleMatForwardRenderPass::SingleMatForwardRenderPass(const SingleMatForwardRenderPassParams& params)
        : SingleMatForwardRenderPass()
    {
      m_params = params;
    }

    void SingleMatForwardRenderPass::Render()
    {
      PUSH_CPU_MARKER("SingleMatForwardRenderPass::Render");

      Renderer* renderer = GetRenderer();

      RenderJobItr begin = m_params.ForwardParams.renderData->GetForwardOpaqueBegin();
      RenderJobItr end   = m_params.ForwardParams.renderData->GetForwardTranslucentBegin();

      for (RenderJobArray::iterator job = begin; begin != end; begin++)
      {
        renderer->Render(*job);
      }

      RenderTranslucent(m_params.ForwardParams.renderData);

      POP_CPU_MARKER();
    }

    void SingleMatForwardRenderPass::PreRender()
    {
      PUSH_CPU_MARKER("SingleMatForwardRenderPass::PreRender");

      ForwardRenderPass::m_params = m_params.ForwardParams;
      ForwardRenderPass::PreRender();

      m_overrideMat->UnInit();
      m_overrideMat->m_fragmentShader = m_params.OverrideFragmentShader;
      m_overrideMat->Init();

      m_program = GetGpuProgramManager()->CreateProgram(m_overrideMat->m_vertexShader, m_overrideMat->m_fragmentShader);
      GetRenderer()->BindProgram(m_program);

      POP_CPU_MARKER();
    };

  } // namespace Editor
} // namespace ToolKit