/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "SingleMaterialPass.h"

#include "Material.h"
#include "TKProfiler.h"

#include "DebugNew.h"

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

      Renderer* renderer      = GetRenderer();
      renderer->m_overrideMat = m_overrideMat;
      for (RenderJob& job : m_params.ForwardParams.OpaqueJobs)
      {
        RenderJobProcessor::SortLights(job, m_params.ForwardParams.Lights);

        MaterialPtr mat = job.Material;
        renderer->m_overrideMat->SetRenderState(mat->GetRenderState());
        renderer->m_overrideMat->m_vertexShader    = mat->m_vertexShader;
        renderer->m_overrideMat->m_fragmentShader  = m_params.OverrideFragmentShader;
        renderer->m_overrideMat->m_diffuseTexture  = mat->m_diffuseTexture;
        renderer->m_overrideMat->m_emissiveTexture = mat->m_emissiveTexture;
        renderer->m_overrideMat->m_emissiveColor   = mat->m_emissiveColor;
        renderer->m_overrideMat->m_cubeMap         = mat->m_cubeMap;
        renderer->m_overrideMat->m_color           = mat->m_color;
        renderer->m_overrideMat->SetAlpha(mat->GetAlpha());
        renderer->m_overrideMat->Init();

        renderer->Render(job, m_params.ForwardParams.Cam, m_params.ForwardParams.Lights);
      }

      RenderTranslucent(m_params.ForwardParams.TranslucentJobs,
                        m_params.ForwardParams.Cam,
                        m_params.ForwardParams.Lights);

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

      POP_CPU_MARKER();
    };

  } // namespace Editor
} // namespace ToolKit