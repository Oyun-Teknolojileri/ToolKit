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

#include "SsaoPass.h"

#include "Camera.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include <gles2.h>

#include "DebugNew.h"

namespace ToolKit
{

  // SSAONoiseTexture
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(SSAONoiseTexture, DataTexture);

  SSAONoiseTexture::SSAONoiseTexture(int width, int height) : DataTexture(width, height) {}

  void SSAONoiseTexture::Init(void* data)
  {
    if (m_initiated)
    {
      return;
    }

    GLint currId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, m_width, m_height, 0, GL_RG, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, currId);

    m_initiated = true;
  }

  SSAONoiseTexture::SSAONoiseTexture() {}

  void SSAONoiseTexture::Init(bool flushClientSideArray)
  {
    assert(false); // The code should never come here
  }

  // SSAOPass
  //////////////////////////////////////////////////////////////////////////

  SSAOPass::SSAOPass()
  {
    m_ssaoFramebuffer = std::make_shared<Framebuffer>();
    m_ssaoTexture     = MakeNewPtr<RenderTarget>();
    m_tempBlurRt      = MakeNewPtr<RenderTarget>();
    m_noiseTexture    = std::make_shared<SSAONoiseTexture>(4, 4);
    m_quadPass        = std::make_shared<FullQuadPass>();
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
    Renderer* renderer = GetRenderer();

    // Generate SSAO texture
    renderer->SetTexture(0, m_params.GPositionBuffer->m_textureId);
    renderer->SetTexture(1, m_params.GNormalBuffer->m_textureId);
    renderer->SetTexture(2, m_noiseTexture->m_textureId);
    renderer->SetTexture(3, m_params.GLinearDepthBuffer->m_textureId);

    m_ssaoShader->SetShaderParameter("radius", ParameterVariant(m_params.Radius));
    m_ssaoShader->SetShaderParameter("bias", ParameterVariant(m_params.Bias));

    RenderSubPass(m_quadPass);

    // Horizontal blur
    renderer->Apply7x1GaussianBlur(m_ssaoTexture, m_tempBlurRt, X_AXIS, 1.0f / m_ssaoTexture->m_width);

    // Vertical blur
    renderer->Apply7x1GaussianBlur(m_tempBlurRt, m_ssaoTexture, Y_AXIS, 1.0f / m_ssaoTexture->m_height);
  }

  void SSAOPass::PreRender()
  {
    Pass::PreRender();

    int width  = m_params.GPositionBuffer->m_width;
    int height = m_params.GPositionBuffer->m_height;

    GenerateSSAONoise();

    // No need destroy and re init framebuffer when size is changed, because
    // the only render target is already being resized.
    m_ssaoFramebuffer->Init({(uint) width, (uint) height, false, false});

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.WarpS               = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT               = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat      = GraphicTypes::FormatR32F;
    oneChannelSet.Format              = GraphicTypes::FormatRed;
    oneChannelSet.Type                = GraphicTypes::TypeFloat;

    // Init ssao texture
    m_ssaoTexture->m_settings         = oneChannelSet;
    m_ssaoTexture->ReconstructIfNeeded((uint) width, (uint) height);

    m_ssaoFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0, m_ssaoTexture);

    // Init temporary blur render target
    m_tempBlurRt->m_settings = oneChannelSet;
    m_tempBlurRt->ReconstructIfNeeded((uint) width, (uint) height);

    // Init noise texture
    m_noiseTexture->Init(&m_ssaoNoise[0]);

    m_quadPass->m_params.FrameBuffer      = m_ssaoFramebuffer;
    m_quadPass->m_params.ClearFrameBuffer = true;

    // SSAO fragment shader
    if (!m_ssaoShader)
    {
      m_ssaoShader = GetShaderManager()->Create<Shader>(ShaderPath("ssaoCalcFrag.shader", true));
    }

    if (m_prevSpread != m_params.spread)
    {
      // Update kernel
      for (uint i = 0; i < 64; ++i)
      {
        m_ssaoShader->SetShaderParameter(g_ssaoSamplesStrCache[i], ParameterVariant(m_ssaoKernel[i]));
      }

      m_prevSpread = m_params.spread;
    }

    m_ssaoShader->SetShaderParameter("screenSize", ParameterVariant(Vec2(width, height)));
    m_ssaoShader->SetShaderParameter("bias", ParameterVariant(m_params.Bias));
    m_ssaoShader->SetShaderParameter("projection", ParameterVariant(m_params.Cam->GetProjectionMatrix()));
    m_ssaoShader->SetShaderParameter("viewMatrix", ParameterVariant(m_params.Cam->GetViewMatrix()));

    m_quadPass->m_params.FragmentShader = m_ssaoShader;
  }

  void SSAOPass::PostRender() { Pass::PostRender(); }

  void SSAOPass::GenerateSSAONoise()
  {
    if (m_ssaoKernel.size() == 0 || m_prevSpread != m_params.spread)
    {
      m_ssaoKernel = GenerateRandomSamplesInHemisphere(64, m_params.spread);
    }

    if (m_ssaoNoise.size() == 0)
    {
      // generates random floats between 0.0 and 1.0
      std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
      std::default_random_engine generator;

      for (unsigned int i = 0; i < 16; i++)
      {
        glm::vec2 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f);
        m_ssaoNoise.push_back(noise);
      }
    }
  }

} // namespace ToolKit