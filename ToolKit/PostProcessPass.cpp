#include "PostProcessPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

namespace ToolKit
{

  BloomPass::BloomPass()
  {
    m_downsampleShader = GetShaderManager()->Create<Shader>(
        ShaderPath("bloomDownsample.shader", true));
    m_upsampleShader = GetShaderManager()->Create<Shader>(
        ShaderPath("bloomUpsample.shader", true));

    m_pass = std::make_shared<FullQuadPass>();
  }

  BloomPass::BloomPass(const BloomPassParams& params) : BloomPass()
  {
    m_params = params;
  }

  BloomPass::~BloomPass() {}

  void BloomPass::Render()
  {
    PreRender();

    RenderTargetPtr mainRt = m_params.FrameBuffer->GetAttachment(
        Framebuffer::Attachment::ColorAttachment0);

    if (mainRt == nullptr || m_invalidRenderParams)
    {
      PostRender();
      return;
    }

    UVec2 mainRes = UVec2(mainRt->m_width, mainRt->m_height);

    // Filter pass
    {
      m_pass->m_params.FragmentShader = m_downsampleShader;
      int passIndx                    = 0;

      m_downsampleShader->SetShaderParameter("passIndx",
                                             ParameterVariant(passIndx));

      m_downsampleShader->SetShaderParameter("srcResolution",
                                             ParameterVariant(mainRes));

      TexturePtr prevRt = m_params.FrameBuffer->GetAttachment(
          Framebuffer::Attachment::ColorAttachment0);

      GetRenderer()->SetTexture(0, prevRt->m_textureId);
      m_pass->m_params.FrameBuffer      = m_tempFrameBuffers[0];
      m_pass->m_params.BlendFunc        = BlendFunction::NONE;
      m_pass->m_params.ClearFrameBuffer = true;
      m_pass->Render();
    }

    // Downsample Pass
    for (int i = 0; i < m_params.iterationCount; i++)
    {
      // Calculate current and previous resolutions

      float powVal = glm::pow(2.0f, float(i + 1));
      const Vec2 factor(1.0f / powVal);
      const UVec2 curRes             = Vec2(mainRes) * factor;

      powVal                         = glm::pow(2.0f, float(i));
      const Vec2 prevRes             = Vec2(mainRes) * Vec2((1.0f / powVal));

      // Find previous framebuffer & RT
      FramebufferPtr prevFramebuffer = m_tempFrameBuffers[i];
      TexturePtr prevRt              = prevFramebuffer->GetAttachment(
          Framebuffer::Attachment::ColorAttachment0);

      // Set pass' shader and parameters
      m_pass->m_params.FragmentShader = m_downsampleShader;

      int passIndx                    = i + 1;
      m_downsampleShader->SetShaderParameter("passIndx",
                                             ParameterVariant(passIndx));
      m_downsampleShader->SetShaderParameter(
          "threshold",
          ParameterVariant(m_params.minThreshold));
      m_downsampleShader->SetShaderParameter("srcResolution",
                                             ParameterVariant(prevRes));

      GetRenderer()->SetTexture(0, prevRt->m_textureId);

      // Set pass parameters
      m_pass->m_params.ClearFrameBuffer = true;
      m_pass->m_params.FrameBuffer      = m_tempFrameBuffers[i + 1];
      m_pass->m_params.BlendFunc        = BlendFunction::NONE;

      m_pass->Render();
    }

    // Upsample Pass
    const float filterRadius = 0.002f;
    for (int i = m_params.iterationCount; i > 0; i--)
    {
      m_pass->m_params.FragmentShader = m_upsampleShader;
      m_upsampleShader->SetShaderParameter("filterRadius",
                                           ParameterVariant(filterRadius));

      FramebufferPtr prevFramebuffer = m_tempFrameBuffers[i];
      TexturePtr prevRt              = prevFramebuffer->GetAttachment(
          Framebuffer::Attachment::ColorAttachment0);
      GetRenderer()->SetTexture(0, prevRt->m_textureId);

      m_pass->m_params.BlendFunc        = BlendFunction::ONE_TO_ONE;
      m_pass->m_params.ClearFrameBuffer = false;
      m_pass->m_params.FrameBuffer      = m_tempFrameBuffers[i - 1];
      m_upsampleShader->SetShaderParameter("intensity", ParameterVariant(1.0f));
      m_pass->Render();
    }

    // Merge Pass
    {
      m_pass->m_params.FragmentShader = m_upsampleShader;
      m_upsampleShader->SetShaderParameter("filterRadius",
                                           ParameterVariant(filterRadius));

      FramebufferPtr prevFramebuffer = m_tempFrameBuffers[0];
      TexturePtr prevRt              = prevFramebuffer->GetAttachment(
          Framebuffer::Attachment::ColorAttachment0);
      GetRenderer()->SetTexture(0, prevRt->m_textureId);

      m_pass->m_params.BlendFunc        = BlendFunction::ONE_TO_ONE;
      m_pass->m_params.ClearFrameBuffer = false;
      m_pass->m_params.FrameBuffer      = m_params.FrameBuffer;
      m_upsampleShader->SetShaderParameter(
          "intensity",
          ParameterVariant(m_params.intensity));

      m_pass->Render();
    }

    PostRender();
  }

  void BloomPass::PreRender()
  {
    Pass::PreRender();

    RenderTargetPtr mainRt = m_params.FrameBuffer->GetAttachment(
        Framebuffer::Attachment::ColorAttachment0);
    if (!mainRt)
    {
      return;
    }

    // Set to minimum iteration count
    Vec2 mainRes = UVec2(mainRt->m_width, mainRt->m_height);
    const IVec2 maxIterCounts(glm::log2(mainRes) - 1.0f);
    m_params.iterationCount =
        glm::min(m_params.iterationCount,
                 glm::min(maxIterCounts.x, maxIterCounts.y));

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

      RenderTargetPtr& rt           = m_tempTextures[i];
      rt                            = std::make_shared<RenderTarget>();
      rt->m_settings.InternalFormat = GraphicTypes::FormatRGB16F;
      rt->m_settings.MagFilter      = GraphicTypes::SampleLinear;
      rt->m_settings.MinFilter      = GraphicTypes::SampleLinear;
      rt->m_settings.WarpR          = GraphicTypes::UVClampToEdge;
      rt->m_settings.WarpS          = GraphicTypes::UVClampToEdge;
      rt->m_settings.WarpT          = GraphicTypes::UVClampToEdge;
      rt->ReconstructIfNeeded(curRes.x, curRes.y);

      FramebufferPtr& fb = m_tempFrameBuffers[i];
      fb                 = std::make_shared<Framebuffer>();
      fb->ReconstructIfNeeded(curRes.x, curRes.y);
      fb->SetAttachment(Framebuffer::Attachment::ColorAttachment0, rt);
    }
  }

  void BloomPass::PostRender() { Pass::PostRender(); }

  PostProcessPass::PostProcessPass()
  {
    m_copyTexture = std::make_shared<RenderTarget>();
    m_copyBuffer  = std::make_shared<Framebuffer>();
    m_copyBuffer->Init({0, 0, false, false});

    m_postProcessPass = std::make_shared<FullQuadPass>();
  }

  PostProcessPass::PostProcessPass(const PostProcessPassParams& params)
      : PostProcessPass()
  {
    m_params = params;
  }

  PostProcessPass::~PostProcessPass() {}

  void PostProcessPass::PreRender()
  {
    Pass::PreRender();

    Renderer* renderer = GetRenderer();

    // Initiate copy buffer.
    FramebufferSettings fbs;
    fbs.depthStencil    = false;
    fbs.useDefaultDepth = false;
    if (m_params.FrameBuffer == nullptr)
    {
      fbs.width  = renderer->m_windowSize.x;
      fbs.height = renderer->m_windowSize.y;
    }
    else
    {
      FramebufferSettings targetFbs = m_params.FrameBuffer->GetSettings();
      fbs.width                     = targetFbs.width;
      fbs.height                    = targetFbs.height;
    }

    m_copyTexture->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->ReconstructIfNeeded(fbs.width, fbs.height);
    m_copyBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                m_copyTexture);

    // Copy given buffer.
    renderer->CopyFrameBuffer(m_params.FrameBuffer,
                              m_copyBuffer,
                              GraphicBitFields::ColorBits);

    // Set given buffer as a texture to be read in gamma pass.
    renderer->SetTexture(0, m_copyTexture->m_textureId);

    m_postProcessPass->m_params.FragmentShader   = m_postProcessShader;
    m_postProcessPass->m_params.FrameBuffer      = m_params.FrameBuffer;
    m_postProcessPass->m_params.ClearFrameBuffer = false;
  }

  void PostProcessPass::Render()
  {
    PreRender();
    m_postProcessPass->Render();
    PostRender();
  }

  void PostProcessPass::PostRender() { Pass::PostRender(); }

  GammaPass::GammaPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(
        ShaderPath("gammaFrag.shader", true));
  }

  GammaPass::GammaPass(const GammaPassParams& params) : GammaPass()
  {
    m_params = params;
  }

  GammaPass::~GammaPass() {}

  void GammaPass::PreRender()
  {
    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter("Gamma",
                                            ParameterVariant(m_params.Gamma));
  }

  TonemapPass::TonemapPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(
        ShaderPath("tonemapFrag.shader", true));
  }

  TonemapPass::TonemapPass(const TonemapPassParams& params) : TonemapPass()
  {
    m_params = params;
  }

  TonemapPass::~TonemapPass() {}

  void TonemapPass::PreRender()
  {
    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter(
        "UseAcesTonemapper",
        ParameterVariant((uint) m_params.Method));
  }

} // namespace ToolKit