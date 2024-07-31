/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "SsaoPass.h"

#include "Camera.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "TKProfiler.h"
#include "TKStats.h"
#include "ToolKit.h"

#include <random>

namespace ToolKit
{

  // SSAOPass
  //////////////////////////////////////////

  StringArray SSAOPass::m_ssaoSamplesStrCache;

  SSAOPass::SSAOPass()
  {
    m_ssaoFramebuffer = MakeNewPtr<Framebuffer>();
    m_ssaoTexture     = MakeNewPtr<RenderTarget>();
    m_tempBlurRt      = MakeNewPtr<RenderTarget>();

    TextureSettings noiseSet;
    noiseSet.InternalFormat = GraphicTypes::FormatRG32F;
    noiseSet.Format         = GraphicTypes::FormatRG;
    noiseSet.Type           = GraphicTypes::TypeFloat;
    m_noiseTexture          = MakeNewPtr<DataTexture>(4, 4, noiseSet);
    m_quadPass              = MakeNewPtr<FullQuadPass>();

    m_ssaoSamplesStrCache.reserve(128);
    for (int i = 0; i < m_ssaoSamplesStrCacheSize; ++i)
    {
      m_ssaoSamplesStrCache.push_back("samples[" + std::to_string(i) + "]");
    }

    m_ssaoShader = GetShaderManager()->Create<Shader>(ShaderPath("ssaoCalcFrag.shader", true));
  }

  SSAOPass::SSAOPass(const SSAOPassParams& params) : SSAOPass() { m_params = params; }

  SSAOPass::~SSAOPass()
  {
    m_ssaoFramebuffer = nullptr;
    m_noiseTexture    = nullptr;
    m_tempBlurRt      = nullptr;
    m_quadPass        = nullptr;
    m_ssaoShader      = nullptr;
  }

  void SSAOPass::Render()
  {
    PUSH_GPU_MARKER("SSAOPass::Render");
    PUSH_CPU_MARKER("SSAOPass::Render");

    Renderer* renderer = GetRenderer();

    // Generate SSAO texture
    renderer->SetTexture(1, m_params.GNormalBuffer->m_textureId);
    renderer->SetTexture(2, m_noiseTexture->m_textureId);
    renderer->SetTexture(3, m_params.GLinearDepthBuffer->m_textureId);

    RenderSubPass(m_quadPass);

    // Horizontal blur
    renderer->Apply7x1GaussianBlur(m_ssaoTexture, m_tempBlurRt, X_AXIS, 1.0f / m_ssaoTexture->m_width);

    // Vertical blur
    renderer->Apply7x1GaussianBlur(m_tempBlurRt, m_ssaoTexture, Y_AXIS, 1.0f / m_ssaoTexture->m_height);

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void SSAOPass::PreRender()
  {
    PUSH_GPU_MARKER("SSAOPass::PreRender");
    PUSH_CPU_MARKER("SSAOPass::PreRender");

    Pass::PreRender();

    int width           = m_params.GNormalBuffer->m_width;
    int height          = m_params.GNormalBuffer->m_height;

    // Clamp kernel size
    m_params.KernelSize = glm::clamp(m_params.KernelSize, m_minimumKernelSize, m_maximumKernelSize);

    GenerateSSAONoise();

    // No need destroy and re init framebuffer when size is changed, because
    // the only render target is already being resized.
    m_ssaoFramebuffer->Init({width, height, false, false});

    TextureSettings oneChannelSet;
    oneChannelSet.WarpS           = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT           = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat  = GraphicTypes::FormatR32F;
    oneChannelSet.Format          = GraphicTypes::FormatRed;
    oneChannelSet.Type            = GraphicTypes::TypeFloat;
    oneChannelSet.GenerateMipMap  = false;

    // Init ssao texture
    m_ssaoTexture->Settings(oneChannelSet);
    m_ssaoTexture->ReconstructIfNeeded(width, height);

    m_ssaoFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, m_ssaoTexture);

    // Init temporary blur render target
    m_tempBlurRt->Settings(oneChannelSet);
    m_tempBlurRt->ReconstructIfNeeded((uint) width, (uint) height);

    m_quadPass->m_params.frameBuffer      = m_ssaoFramebuffer;
    m_quadPass->m_params.clearFrameBuffer = GraphicBitFields::None;

    m_quadPass->SetFragmentShader(m_ssaoShader, GetRenderer());

    if (m_params.KernelSize != m_currentKernelSize || m_prevSpread != m_params.spread)
    {
      // Update kernel
      for (int i = 0; i < m_params.KernelSize; ++i)
      {
        m_quadPass->UpdateUniform(ShaderUniform(m_ssaoSamplesStrCache[i], m_ssaoKernel[i]));
      }

      m_prevSpread = m_params.spread;
    }

    m_quadPass->UpdateUniform(ShaderUniform("screenSize", Vec2(width, height)));
    m_quadPass->UpdateUniform(ShaderUniform("bias", m_params.Bias));
    m_quadPass->UpdateUniform(ShaderUniform("kernelSize", m_params.KernelSize));
    m_quadPass->UpdateUniform(ShaderUniform("projection", m_params.Cam->GetProjectionMatrix()));
    m_quadPass->UpdateUniform(ShaderUniform("viewMatrix", m_params.Cam->GetViewMatrix()));
    m_quadPass->UpdateUniform(ShaderUniform("radius", m_params.Radius));
    m_quadPass->UpdateUniform(ShaderUniform("bias", m_params.Bias));

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void SSAOPass::PostRender()
  {
    PUSH_GPU_MARKER("SSAOPass::PostRender");
    PUSH_CPU_MARKER("SSAOPass::PostRender");

    m_currentKernelSize = m_params.KernelSize;

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void SSAOPass::GenerateSSAONoise()
  {
    CPU_FUNC_RANGE();

    if (m_prevSpread != m_params.spread)
    {
      GenerateRandomSamplesInHemisphere(m_maximumKernelSize, m_params.spread, m_ssaoKernel);
    }

    if (m_ssaoNoise.size() == 0)
    {
      // generates random floats between 0.0 and 1.0
      std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
      std::default_random_engine generator;

      for (unsigned int i = 0; i < 16; i++)
      {
        Vec2 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f);
        m_ssaoNoise.push_back(noise);
      }

      // Init noise texture.
      m_noiseTexture->UnInit();
      m_noiseTexture->Init(m_ssaoNoise.data());
    }
  }

} // namespace ToolKit