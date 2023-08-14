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

#include "LightingPass.h"

#include "DataTexture.h"
#include "DirectionComponent.h"
#include "Material.h"
#include "Shader.h"

namespace ToolKit
{
  using FAttachment = Framebuffer::Attachment;

  LightingPass::LightingPass()
  {
    m_fullQuadPass        = std::make_shared<FullQuadPass>();
    m_lightingFrameBuffer = std::make_shared<Framebuffer>();
    m_lightingRt          = std::make_shared<RenderTarget>();
    m_lightingShader      = GetShaderManager()->Create<Shader>(ShaderPath("newDeferredFrag.shader", true));
    m_mergeShader         = GetShaderManager()->Create<Shader>(ShaderPath("lightMerge.shader", true));
  }

  LightingPass::~LightingPass() {}

  void LightingPass::Init(const LightingPassParams& params) {}

  void LightingPass::PreRender()
  {
    Pass::PreRender();

    int width  = m_params.MainFramebuffer->GetAttachment(FAttachment::ColorAttachment0)->m_width;
    int height = m_params.MainFramebuffer->GetAttachment(FAttachment::ColorAttachment0)->m_height;

    m_lightingFrameBuffer->Init({(uint) width, (uint) height, false, false});
    m_lightingFrameBuffer->ReconstructIfNeeded((uint) width, (uint) height);

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.WarpS               = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT               = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat      = GraphicTypes::FormatRGB8;
    oneChannelSet.Format              = GraphicTypes::FormatRGB;
    oneChannelSet.Type                = GraphicTypes::TypeUnsignedByte;

    m_lightingRt->m_settings          = oneChannelSet;
    m_lightingRt->ReconstructIfNeeded((uint) width, (uint) height);
    m_lightingFrameBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, m_lightingRt);

    Renderer* renderer          = GetRenderer();
    // Set gbuffer
    // 9: Position, 10: Normal, 11: Color, 14: metallic-roughness,
    FramebufferPtr gFrameBuffer = m_params.GBufferFramebuffer;

    renderer->SetTexture(9, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment0)->m_textureId);
    renderer->SetTexture(10, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment1)->m_textureId);
    renderer->SetTexture(11, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment2)->m_textureId);
    renderer->SetTexture(14, gFrameBuffer->GetAttachment(FAttachment::ColorAttachment5)->m_textureId);

    m_lightingShader->SetShaderParameter("aoEnabled", ParameterVariant(m_params.AOTexture != nullptr));
    renderer->SetTexture(5, m_params.AOTexture ? m_params.AOTexture->m_textureId : 0);
  }

  void LightingPass::SetLightUniforms(Light* light, int lightType)
  {
    Vec3 pos = light->m_node->GetTranslation();
    m_lightingShader->SetShaderParameter("lightType", ParameterVariant(lightType));
    m_lightingShader->SetShaderParameter("lightPos", ParameterVariant(pos));
    m_lightingShader->SetShaderParameter("lightIntensity", ParameterVariant(light->GetIntensityVal()));
    m_lightingShader->SetShaderParameter("lightColor", ParameterVariant(light->GetColorVal()));

    Vec3 dir;
    switch (lightType)
    {
    case 0: // is directional
    case 3:
      dir = static_cast<DirectionalLight*>(light)->GetComponent<DirectionComponent>()->GetDirection();
      break;
    case 1: // is point
    case 4:
    {
      PointLight* pointLight = static_cast<PointLight*>(light);
      m_lightingShader->SetShaderParameter("lightRadius", ParameterVariant(pointLight->GetRadiusVal()));
    }
    break;
    case 2: // is spot
    case 5:
    {
      SpotLight* spotLight = static_cast<SpotLight*>(light);
      dir                  = spotLight->GetComponent<DirectionComponent>()->GetDirection();
      float outAngle       = glm::cos(glm::radians(spotLight->GetOuterAngleVal() * 0.5f));
      float innAngle       = glm::cos(glm::radians(spotLight->GetInnerAngleVal() * 0.5f));
      m_lightingShader->SetShaderParameter("lightRadius", ParameterVariant(spotLight->GetRadiusVal()));
      m_lightingShader->SetShaderParameter("lightOutAngle", ParameterVariant(outAngle));
      m_lightingShader->SetShaderParameter("lightInnAngle", ParameterVariant(innAngle));
    }
    break;
    default:
      assert(0 && "unknown type");
      break;
    }

    m_lightingShader->SetShaderParameter("lightDir", ParameterVariant(dir));

    if (lightType < 3) // does not have shadow
    {
      return;
    }

    float atlasTextureSize = (float) Renderer::m_rhiSettings::g_shadowAtlasTextureSize;
    float bias             = light->GetShadowBiasVal() * Renderer::g_shadowBiasMultiplier;
    const Mat4& projView   = light->m_shadowMapCameraProjectionViewMatrix;
    float atlasResRatio    = light->GetShadowResVal() / atlasTextureSize;

    m_lightingShader->SetShaderParameter("lightShadowMapCameraFar", ParameterVariant(light->m_shadowMapCameraFar));
    m_lightingShader->SetShaderParameter("lightProjectionViewMatrix", ParameterVariant(projView));
    m_lightingShader->SetShaderParameter("lightShadowAtlasCoord", ParameterVariant(light->m_shadowAtlasCoord / atlasTextureSize));
    m_lightingShader->SetShaderParameter("lightShadowAtlasResRatio", ParameterVariant(atlasResRatio));
    m_lightingShader->SetShaderParameter("lightShadowAtlasLayer", ParameterVariant((float) light->m_shadowAtlasLayer));
    m_lightingShader->SetShaderParameter("lightPCFSamples", ParameterVariant(light->GetPCFSamplesVal()));
    m_lightingShader->SetShaderParameter("lightPCFRadius", ParameterVariant(light->GetPCFRadiusVal()));
    m_lightingShader->SetShaderParameter("lightBleedReduction", ParameterVariant(light->GetBleedingReductionVal()));
    m_lightingShader->SetShaderParameter("lightShadowBias", ParameterVariant(bias));
  }

  void LightingPass::Render()
  {
    Renderer* renderer = GetRenderer();
    renderer->SetFramebuffer(m_lightingFrameBuffer, true, Vec4(0.0f));
    // Deferred render always uses PBR material
    m_fullQuadPass->m_material->m_materialType = MaterialType::PBR;
    m_fullQuadPass->m_params.BlendFunc         = BlendFunction::ONE_TO_ONE; // additive blending
    m_fullQuadPass->m_params.FrameBuffer       = m_lightingFrameBuffer;
    m_fullQuadPass->m_params.FragmentShader    = m_lightingShader;
    m_fullQuadPass->m_params.ClearFrameBuffer  = false;
    
    m_lightingShader->SetShaderParameter("camPos", ParameterVariant(m_params.Cam->m_node->GetTranslation()));

    // Add all lights to a texture
    for (int i = 0; i < m_params.lights.size(); i++)
    {
      Light* light  = m_params.lights[i];
      int hasShadow = light->GetCastShadowVal() * 3; // if light has shadow index will start from 3
      int lightType = ((int) light->GetType() - (int) EntityType::Entity_DirectionalLight) + hasShadow;
      
      assert(lightType < 6 && lightType >= 0 && "light type invalid");
      SetLightUniforms(light, lightType);
      RenderSubPass(m_fullQuadPass);
    }

    // prepare to write main frame buffer
    m_fullQuadPass->m_params.FragmentShader   = m_mergeShader;
    m_fullQuadPass->m_params.FrameBuffer      = m_params.MainFramebuffer;
    m_fullQuadPass->m_params.ClearFrameBuffer = false;
    m_fullQuadPass->m_params.BlendFunc        = BlendFunction::NONE;

    RenderTargetPtr emmisiveRt = m_params.GBufferFramebuffer->GetAttachment(FAttachment::ColorAttachment3);
    RenderTargetPtr iblRt      = m_params.GBufferFramebuffer->GetAttachment(FAttachment::ColorAttachment6);

    renderer->SetTexture(0, m_lightingRt->m_textureId);
    renderer->SetTexture(1, emmisiveRt->m_textureId);
    renderer->SetTexture(2, iblRt->m_textureId);

    m_mergeShader->SetShaderParameter("aoEnabled", ParameterVariant(m_params.AOTexture != nullptr));
    renderer->SetTexture(5, m_params.AOTexture != nullptr ? m_params.AOTexture->m_textureId : 0);
    // merge lighting, ibl, ao, and emmisive
    RenderSubPass(m_fullQuadPass);
  }

  void LightingPass::PostRender()
  {
    // swap depth texture of gbuffer and this. because main depth buffer is empty now
    DepthTexturePtr mainDepth = m_params.MainFramebuffer->GetDepthTexture();
    m_params.MainFramebuffer->AttachDepthTexture(m_params.GBufferFramebuffer->GetDepthTexture());
    m_params.GBufferFramebuffer->AttachDepthTexture(mainDepth);
    Pass::PostRender();
  }
} // namespace ToolKit