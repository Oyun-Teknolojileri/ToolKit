/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "BloomPass.h"

#include "Shader.h"
#include "Stats.h"
#include "ToolKit.h"

namespace ToolKit
{

  BloomPass::BloomPass() : Pass("BloomPass")
  {
    m_downsampleShader = GetShaderManager()->Create<Shader>(ShaderPath("bloomDownsample.shader", true));
    m_upsampleShader   = GetShaderManager()->Create<Shader>(ShaderPath("bloomUpsample.shader", true));
    m_pass             = MakeNewPtr<FullQuadPass>();
  }

  void BloomPass::Render()
  {
    RenderTargetPtr mainRt = m_params.FrameBuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);

    if (mainRt == nullptr || m_invalidRenderParams)
    {
      return;
    }

    UVec2 mainRes      = UVec2(mainRt->m_width, mainRt->m_height);

    Renderer* renderer = GetRenderer();

    // Filter pass
    {
      m_pass->SetFragmentShader(m_downsampleShader, renderer);
      int passIndx = 0;

      m_pass->UpdateUniform(ShaderUniform("passIndx", passIndx));
      m_pass->UpdateUniform(ShaderUniform("srcResolution", mainRes));
      m_pass->UpdateUniform(ShaderUniform("threshold", m_params.minThreshold));

      TexturePtr prevRt = m_params.FrameBuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);

      renderer->SetTexture(0, prevRt->m_textureId);
      m_pass->m_params.frameBuffer      = m_tempFrameBuffers[0];
      m_pass->m_params.blendFunc        = BlendFunction::NONE;
      m_pass->m_params.clearFrameBuffer = GraphicBitFields::AllBits;

      RenderSubPass(m_pass);
    }

    // Downsample Pass
    {
      m_pass->SetFragmentShader(m_downsampleShader, renderer);

      for (int i = 0; i < m_currentIterationCount; i++)
      {
        // Calculate current and previous resolutions

        float powVal = glm::pow(2.0f, float(i + 1));
        const Vec2 factor(1.0f / powVal);
        const UVec2 curRes             = Vec2(mainRes) * factor;

        powVal                         = glm::pow(2.0f, float(i));
        const Vec2 prevRes             = Vec2(mainRes) * Vec2((1.0f / powVal));

        // Find previous framebuffer & RT
        FramebufferPtr prevFramebuffer = m_tempFrameBuffers[i];
        TexturePtr prevRt              = prevFramebuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);

        // Set pass' shader and parameters

        int passIndx                   = i + 1;

        m_pass->UpdateUniform(ShaderUniform("passIndx", passIndx));
        m_pass->UpdateUniform(ShaderUniform("srcResolution", prevRes));

        renderer->SetTexture(0, prevRt->m_textureId);

        // Set pass parameters
        m_pass->m_params.clearFrameBuffer = GraphicBitFields::AllBits;
        m_pass->m_params.frameBuffer      = m_tempFrameBuffers[i + 1];
        m_pass->m_params.blendFunc        = BlendFunction::NONE;

        RenderSubPass(m_pass);
      }
    }

    // Upsample Pass
    {
      m_pass->SetFragmentShader(m_upsampleShader, renderer);

      const float filterRadius = 0.002f;
      m_pass->UpdateUniform(ShaderUniform("filterRadius", filterRadius));
      m_pass->UpdateUniform(ShaderUniform("intensity", 1.0f));

      for (int i = m_currentIterationCount; i > 0; i--)
      {

        FramebufferPtr prevFramebuffer = m_tempFrameBuffers[i];
        TexturePtr prevRt              = prevFramebuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);
        renderer->SetTexture(0, prevRt->m_textureId);

        m_pass->m_params.blendFunc        = BlendFunction::ONE_TO_ONE;
        m_pass->m_params.clearFrameBuffer = GraphicBitFields::None;
        m_pass->m_params.frameBuffer      = m_tempFrameBuffers[i - 1];

        RenderSubPass(m_pass);
      }
    }

    // Merge Pass
    {
      m_pass->SetFragmentShader(m_upsampleShader, renderer);

      FramebufferPtr prevFramebuffer = m_tempFrameBuffers[0];
      TexturePtr prevRt              = prevFramebuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);
      renderer->SetTexture(0, prevRt->m_textureId);

      m_pass->m_params.blendFunc        = BlendFunction::ONE_TO_ONE;
      m_pass->m_params.clearFrameBuffer = GraphicBitFields::None;
      m_pass->m_params.frameBuffer      = m_params.FrameBuffer;

      m_pass->UpdateUniform(ShaderUniform("intensity", m_params.intensity));

      RenderSubPass(m_pass);
    }
  }

  void BloomPass::PreRender()
  {
    Pass::PreRender();

    RenderTargetPtr mainRt = m_params.FrameBuffer->GetColorAttachment(Framebuffer::Attachment::ColorAttachment0);
    if (!mainRt)
    {
      return;
    }

    // Set to minimum iteration count
    Vec2 mainRes = UVec2(mainRt->m_width, mainRt->m_height);
    const IVec2 maxIterCounts(glm::log2(mainRes) - 1.0f);
    int iterationCount = glm::min(m_params.iterationCount, glm::min(maxIterCounts.x, maxIterCounts.y));

    if (iterationCount < 0)
    {
      m_invalidRenderParams = true;
      return;
    }

    if (iterationCount != m_currentIterationCount)
    {
      m_tempTextures.resize(m_params.iterationCount + 1);
      m_tempFrameBuffers.resize(m_params.iterationCount + 1);

      for (int i = 0; i < m_params.iterationCount + 1; i++)
      {
        const Vec2 factor(1.0f / glm::pow(2.0f, float(i)));
        const UVec2 curRes    = Vec2(mainRes) * factor;

        m_invalidRenderParams = false;
        if (curRes.x == 1 || curRes.y == 1)
        {
          m_invalidRenderParams = true;
          return;
        }

        RenderTargetPtr& rt = m_tempTextures[i];

        TextureSettings set;
        set.InternalFormat = GraphicTypes::FormatRGBA16F;
        set.Format         = GraphicTypes::FormatRGBA;
        set.Type           = GraphicTypes::TypeFloat;
        set.MagFilter      = GraphicTypes::SampleLinear;
        set.MinFilter      = GraphicTypes::SampleLinear;
        set.WarpR          = GraphicTypes::UVClampToEdge;
        set.WarpS          = GraphicTypes::UVClampToEdge;
        set.WarpT          = GraphicTypes::UVClampToEdge;
        set.GenerateMipMap = false;

        rt                 = MakeNewPtr<RenderTarget>(curRes.x, curRes.y, set);
        rt->Init();

        FramebufferPtr& frameBuffer = m_tempFrameBuffers[i];
        if (frameBuffer == nullptr)
        {
          FramebufferSettings fbSettings;
          fbSettings.depthStencil    = false;
          fbSettings.useDefaultDepth = false;
          fbSettings.width           = curRes.x;
          fbSettings.height          = curRes.y;

          frameBuffer                = MakeNewPtr<Framebuffer>(fbSettings, "BloomDownSampleFB");
          frameBuffer->Init();
        }

        frameBuffer->ReconstructIfNeeded(curRes.x, curRes.y);
        frameBuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, rt);
      }

      m_currentIterationCount = iterationCount;
    }
  }

  void BloomPass::PostRender() { Pass::PostRender(); }

} // namespace ToolKit