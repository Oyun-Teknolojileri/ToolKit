#include "PostProcessPass.h"

#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include <random>

#include "DebugNew.h"

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

  BloomPass::~BloomPass()
  {
    m_pass             = nullptr;
    m_downsampleShader = nullptr;
    m_upsampleShader   = nullptr;
    m_tempTextures.clear();
  }

  void BloomPass::Render()
  {
    RenderTargetPtr mainRt = m_params.FrameBuffer->GetAttachment(
        Framebuffer::Attachment::ColorAttachment0);

    if (mainRt == nullptr || m_invalidRenderParams)
    {
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

      RenderSubPass(m_pass);
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

      RenderSubPass(m_pass);
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

      RenderSubPass(m_pass);
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

      RenderSubPass(m_pass);
    }
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

  PostProcessPass::~PostProcessPass()
  {
    m_postProcessShader = nullptr;
    m_postProcessPass   = nullptr;
    m_copyBuffer        = nullptr;
    m_copyTexture       = nullptr;
  }

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

  void PostProcessPass::Render() { RenderSubPass(m_postProcessPass); }

  void PostProcessPass::PostRender() { Pass::PostRender(); }

  FXAAPass::FXAAPass() : PostProcessPass()
  {
    m_postProcessShader = GetShaderManager()->Create<Shader>(
      ShaderPath("fxaaFilterFrag.shader", true));
  }

  FXAAPass::FXAAPass(const FXAAPassParams& params) : FXAAPass()
  {
    m_params = params;
  }

  void FXAAPass::PreRender()
  {
    PostProcessPass::m_params.FrameBuffer = m_params.FrameBuffer;
    PostProcessPass::PreRender();

    m_postProcessShader->SetShaderParameter("screen_size",
      ParameterVariant(m_params.screen_size));
  }

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

  SSAOPass::SSAOPass()
  {
    m_ssaoFramebuffer = std::make_shared<Framebuffer>();
    m_ssaoTexture     = std::make_shared<RenderTarget>();
    m_tempBlurRt      = std::make_shared<RenderTarget>();
    m_noiseTexture    = std::make_shared<SSAONoiseTexture>(4, 4);
    m_quadPass        = std::make_shared<FullQuadPass>();
  }

  SSAOPass::SSAOPass(const SSAOPassParams& params) : SSAOPass()
  {
    m_params = params;
  }

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

    RenderSubPass(m_quadPass);

    // Horizontal blur
    renderer->ApplyAverageBlur(m_ssaoTexture,
                               m_tempBlurRt,
                               X_AXIS,
                               1.0f / m_ssaoTexture->m_width);

    // Vertical blur
    renderer->ApplyAverageBlur(m_tempBlurRt,
                               m_ssaoTexture,
                               Y_AXIS,
                               1.0f / m_ssaoTexture->m_height);
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

    m_ssaoFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                     m_ssaoTexture);

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
      m_ssaoShader = GetShaderManager()->Create<Shader>(
          ShaderPath("ssaoCalcFrag.shader", true));
      for (uint i = 0; i < 64; ++i)
      {
        m_ssaoShader->SetShaderParameter(g_ssaoSamplesStrCache[i],
                                         ParameterVariant(m_ssaoKernel[i]));
      }
    }
    m_ssaoShader->SetShaderParameter("screen_size",
                                     ParameterVariant(Vec2(width, height)));
    m_ssaoShader->SetShaderParameter(
        "projection",
        ParameterVariant(m_params.Cam->GetProjectionMatrix()));
    m_ssaoShader->SetShaderParameter(
        "viewMatrix",
        ParameterVariant(m_params.Cam->GetViewMatrix()));

    m_quadPass->m_params.FragmentShader = m_ssaoShader;
  }

  void SSAOPass::PostRender() { Pass::PostRender(); }

  void SSAOPass::GenerateSSAONoise()
  {
    // generate sample kernel
    // ----------------------
    auto lerp = [](float a, float b, float f) { return a + f * (b - a); };

    // generates random floats between 0.0 and 1.0
    std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
    std::default_random_engine generator;
    if (m_ssaoKernel.size() == 0)
    {
      for (uint i = 0; i < 64; ++i)
      {
        Vec3 sample(randomFloats(generator) * 2.0f - 1.0f,
                    randomFloats(generator) * 2.0f - 1.0f,
                    randomFloats(generator));
        sample      = glm::normalize(sample);
        sample      *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale       = lerp(0.1f, 1.0f, scale * scale);
        sample      *= scale;
        m_ssaoKernel.push_back(sample);
      }
    }

    // generate noise texture
    // ----------------------
    if (m_ssaoNoise.size() == 0)
    {
      for (unsigned int i = 0; i < 16; i++)
      {
        glm::vec2 noise(randomFloats(generator) * 2.0f - 1.0f,
                        randomFloats(generator) * 2.0f - 1.0f);
        m_ssaoNoise.push_back(noise);
      }
    }
  }

} // namespace ToolKit