/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "BloomPass.h"

#include "Shader.h"
#include "TKProfiler.h"
#include "TKStats.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  BloomPass::BloomPass()
  {
    m_downsampleShader = GetShaderManager()->Create<Shader>(ShaderPath("bloomDownsample.shader", true));
    m_upsampleShader   = GetShaderManager()->Create<Shader>(ShaderPath("bloomUpsample.shader", true));
    m_pass             = MakeNewPtr<FullQuadPass>();
  }

  BloomPass::BloomPass(const BloomPassParams& params) : BloomPass() { m_params = params; }

  BloomPass::~BloomPass()
  {
    m_pass             = nullptr;
    m_downsampleShader = nullptr;
    m_upsampleShader   = nullptr;
    m_tempTextures.clear();
  }

  void BloomPass::Render()
  {
    PUSH_GPU_MARKER("BloomPass::Render");
    PUSH_CPU_MARKER("BloomPass::Render");

    RenderTargetPtr mainRt = m_params.FrameBuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);

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

      m_pass->UpdateCustomUniform(ShaderUniform("passIndx", passIndx));
      m_pass->UpdateCustomUniform(ShaderUniform("srcResolution", mainRes));
      m_pass->UpdateCustomUniform(ShaderUniform("threshold", m_params.minThreshold));

      TexturePtr prevRt = m_params.FrameBuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);

      renderer->SetTexture(0, prevRt->m_textureId);
      m_pass->m_params.FrameBuffer      = m_tempFrameBuffers[0];
      m_pass->m_params.BlendFunc        = BlendFunction::NONE;
      m_pass->m_params.ClearFrameBuffer = true;

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
        TexturePtr prevRt              = prevFramebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);

        // Set pass' shader and parameters

        int passIndx                   = i + 1;

        m_pass->UpdateCustomUniform(ShaderUniform("passIndx", passIndx));
        m_pass->UpdateCustomUniform(ShaderUniform("srcResolution", prevRes));

        renderer->SetTexture(0, prevRt->m_textureId);

        // Set pass parameters
        m_pass->m_params.ClearFrameBuffer = true;
        m_pass->m_params.FrameBuffer      = m_tempFrameBuffers[i + 1];
        m_pass->m_params.BlendFunc        = BlendFunction::NONE;

        RenderSubPass(m_pass);
      }
    }

    // Upsample Pass
    {
      m_pass->SetFragmentShader(m_upsampleShader, renderer);

      const float filterRadius = 0.002f;
      m_pass->UpdateCustomUniform(ShaderUniform("filterRadius", filterRadius));
      m_pass->UpdateCustomUniform(ShaderUniform("intensity", 1.0f));

      for (int i = m_currentIterationCount; i > 0; i--)
      {

        FramebufferPtr prevFramebuffer = m_tempFrameBuffers[i];
        TexturePtr prevRt              = prevFramebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);
        renderer->SetTexture(0, prevRt->m_textureId);

        m_pass->m_params.BlendFunc        = BlendFunction::ONE_TO_ONE;
        m_pass->m_params.ClearFrameBuffer = false;
        m_pass->m_params.FrameBuffer      = m_tempFrameBuffers[i - 1];

        RenderSubPass(m_pass);
      }
    }

    // Merge Pass
    {
      m_pass->SetFragmentShader(m_upsampleShader, renderer);

      FramebufferPtr prevFramebuffer = m_tempFrameBuffers[0];
      TexturePtr prevRt              = prevFramebuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);
      renderer->SetTexture(0, prevRt->m_textureId);

      m_pass->m_params.BlendFunc        = BlendFunction::ONE_TO_ONE;
      m_pass->m_params.ClearFrameBuffer = false;
      m_pass->m_params.FrameBuffer      = m_params.FrameBuffer;

      m_pass->UpdateCustomUniform(ShaderUniform("intensity", m_params.intensity));

      RenderSubPass(m_pass);
    }

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void BloomPass::PreRender()
  {
    PUSH_GPU_MARKER("BloomPass::PreRender");
    PUSH_CPU_MARKER("BloomPass::PreRender");

    Pass::PreRender();

    RenderTargetPtr mainRt = m_params.FrameBuffer->GetAttachment(Framebuffer::Attachment::ColorAttachment0);
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

        FramebufferPtr& fb = m_tempFrameBuffers[i];
        if (fb == nullptr)
        {
          FramebufferSettings fbSettings;
          fbSettings.depthStencil    = false;
          fbSettings.useDefaultDepth = false;
          fbSettings.width           = curRes.x;
          fbSettings.height          = curRes.y;
          fb                         = MakeNewPtr<Framebuffer>();
          fb->Init(fbSettings);
        }
        fb->ReconstructIfNeeded(curRes.x, curRes.y);
        fb->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, rt);
      }

      m_currentIterationCount = iterationCount;
    }

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

  void BloomPass::PostRender()
  {
    PUSH_GPU_MARKER("BloomPass::PostRender");
    PUSH_CPU_MARKER("BloomPass::PostRender");

    Pass::PostRender();

    POP_CPU_MARKER();
    POP_GPU_MARKER();
  }

} // namespace ToolKit