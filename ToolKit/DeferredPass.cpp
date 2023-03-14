#include "DeferredPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "FullQuadPass.h"

namespace ToolKit
{

  DeferredRenderPass::DeferredRenderPass()
  {
    m_fullQuadPass = std::make_shared<FullQuadPass>();
  }

  DeferredRenderPass::DeferredRenderPass(const DeferredRenderPassParams& params)
      : DeferredRenderPass()
  {
    m_params = params;
  }

  DeferredRenderPass::~DeferredRenderPass()
  {
    m_fullQuadPass         = nullptr;
    m_deferredRenderShader = nullptr;
    m_lightDataTexture     = nullptr;
  }

  void DeferredRenderPass::PreRender()
  {
    Pass::PreRender();

    if (m_lightDataTexture == nullptr)
    {
      InitLightDataTexture();
    }

    if (m_deferredRenderShader == nullptr)
    {
      m_deferredRenderShader = GetShaderManager()->Create<Shader>(
          ShaderPath("deferredRenderFrag.shader", true));
    }
    m_deferredRenderShader->SetShaderParameter(
        "camPos",
        ParameterVariant(m_params.Cam->m_node->GetTranslation(
            TransformationSpace::TS_WORLD)));

    m_fullQuadPass->m_params.ClearFrameBuffer = m_params.ClearFramebuffer;
    m_fullQuadPass->m_params.FragmentShader   = m_deferredRenderShader;
    m_fullQuadPass->m_params.FrameBuffer      = m_params.MainFramebuffer;

    Renderer* renderer                        = GetRenderer();

    Vec2 sd, nsd, sp, nsp, ss, nss;
    float sizeD, sizeP, sizeS, sizeND, sizeNP, sizeNS;

    // Update light data texture
    m_lightDataTexture->UpdateTextureData(m_params.lights,
                                          sd,
                                          sp,
                                          ss,
                                          nsd,
                                          nsp,
                                          nss,
                                          sizeD,
                                          sizeP,
                                          sizeS,
                                          sizeND,
                                          sizeNP,
                                          sizeNS);

    // Update light uniforms
    m_deferredRenderShader->SetShaderParameter(
        "lightDataTextureWidth",
        ParameterVariant((float) m_lightDataTextureSize.x));
    m_deferredRenderShader->SetShaderParameter("shadowDirLightsInterval",
                                               ParameterVariant(sd));
    m_deferredRenderShader->SetShaderParameter("shadowPointLightsInterval",
                                               ParameterVariant(sp));
    m_deferredRenderShader->SetShaderParameter("shadowSpotLightsInterval",
                                               ParameterVariant(ss));
    m_deferredRenderShader->SetShaderParameter("nonShadowDirLightsInterval",
                                               ParameterVariant(nsd));
    m_deferredRenderShader->SetShaderParameter("nonShadowPointLightsInterval",
                                               ParameterVariant(nsp));
    m_deferredRenderShader->SetShaderParameter("nonShadowSpotLightsInterval",
                                               ParameterVariant(nss));
    m_deferredRenderShader->SetShaderParameter("dirShadowLightDataSize",
                                               ParameterVariant(sizeD));
    m_deferredRenderShader->SetShaderParameter("pointShadowLightDataSize",
                                               ParameterVariant(sizeP));
    m_deferredRenderShader->SetShaderParameter("spotShadowLightDataSize",
                                               ParameterVariant(sizeS));
    m_deferredRenderShader->SetShaderParameter("dirNonShadowLightDataSize",
                                               ParameterVariant(sizeND));
    m_deferredRenderShader->SetShaderParameter("pointNonShadowLightDataSize",
                                               ParameterVariant(sizeNP));
    m_deferredRenderShader->SetShaderParameter("spotNonShadowLightDataSize",
                                               ParameterVariant(sizeNS));

    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color, 12: emissive, 14: metallic-roughness,
    // 16: ibl contribution
    renderer->SetTexture(
        9,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment0)
            ->m_textureId);
    renderer->SetTexture(
        10,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment1)
            ->m_textureId);
    renderer->SetTexture(
        11,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment2)
            ->m_textureId);
    renderer->SetTexture(
        12,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment3)
            ->m_textureId);
    renderer->SetTexture(
        14,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment5)
            ->m_textureId);
    renderer->SetTexture(
        16,
        m_params.GBufferFramebuffer
            ->GetAttachment(Framebuffer::Attachment::ColorAttachment6)
            ->m_textureId);

    renderer->SetTexture(13, m_lightDataTexture->m_textureId);

    if (m_params.AOTexture)
    {
      m_deferredRenderShader->SetShaderParameter("aoEnabled",
                                                 ParameterVariant(1));
      renderer->SetTexture(5, m_params.AOTexture->m_textureId);
    }
    else
    {
      m_deferredRenderShader->SetShaderParameter("aoEnabled",
                                                 ParameterVariant(0));
    }
  }

  void DeferredRenderPass::PostRender()
  {
    // Copy real depth buffer to main framebuffer depth
    GetRenderer()->CopyFrameBuffer(m_params.GBufferFramebuffer,
                                   m_params.MainFramebuffer,
                                   GraphicBitFields::DepthBits);

    Pass::PostRender();
  }

  void DeferredRenderPass::Render()
  {
    // Deferred render always uses PBR material
    m_fullQuadPass->m_material->m_materialType = MaterialType::PBR;
    RenderSubPass(m_fullQuadPass);
  }

  void DeferredRenderPass::InitLightDataTexture()
  {
    m_lightDataTexture =
        std::make_shared<LightDataTexture>(m_lightDataTextureSize.x,
                                           m_lightDataTextureSize.y);
    m_lightDataTexture->Init();
  }

} // namespace ToolKit