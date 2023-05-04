#include "SingleMaterialPass.h"

#include "Material.h"

#include "DebugNew.h"

namespace ToolKit
{
  namespace Editor
  {

    SingleMatForwardRenderPass::SingleMatForwardRenderPass()
        : ForwardRenderPass()
    {
      m_overrideMat = std::make_shared<Material>();
    }

    SingleMatForwardRenderPass::SingleMatForwardRenderPass(
        const SingleMatForwardRenderPassParams& params)
        : SingleMatForwardRenderPass()
    {
      m_params = params;
    }

    void SingleMatForwardRenderPass::Render()
    {

      Renderer* renderer      = GetRenderer();
      renderer->m_overrideMat = std::make_shared<Material>();
      for (RenderJob& job : m_params.ForwardParams.OpaqueJobs)
      {
        LightRawPtrArray lightList =
            RenderJobProcessor::SortLights(job, m_params.ForwardParams.Lights);

        MaterialPtr mat = job.Material;
        renderer->m_overrideMat->SetRenderState(mat->GetRenderState());
        renderer->m_overrideMat->m_vertexShader = mat->m_vertexShader;
        renderer->m_overrideMat->m_fragmentShader =
            m_params.OverrideFragmentShader;
        renderer->m_overrideMat->m_diffuseTexture  = mat->m_diffuseTexture;
        renderer->m_overrideMat->m_emissiveTexture = mat->m_emissiveTexture;
        renderer->m_overrideMat->m_emissiveColor   = mat->m_emissiveColor;
        renderer->m_overrideMat->m_cubeMap         = mat->m_cubeMap;
        renderer->m_overrideMat->m_color           = mat->m_color;
        renderer->m_overrideMat->SetAlpha(mat->GetAlpha());
        renderer->m_overrideMat->Init();

        renderer->Render(job, m_params.ForwardParams.Cam, lightList);
      }

      RenderTranslucent(m_params.ForwardParams.TranslucentJobs,
                        m_params.ForwardParams.Cam,
                        m_params.ForwardParams.Lights);
    }

    void SingleMatForwardRenderPass::PreRender()
    {
      ForwardRenderPass::m_params = m_params.ForwardParams;
      ForwardRenderPass::PreRender();

      m_overrideMat->UnInit();
      m_overrideMat->m_fragmentShader = m_params.OverrideFragmentShader;
      m_overrideMat->Init();
    };

  } // namespace Editor
} // namespace ToolKit