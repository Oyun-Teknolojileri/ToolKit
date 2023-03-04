#include "SsaoPass.h"

#include "Material.h"
#include "Mesh.h"
#include "ShaderReflectionCache.h"
#include "ToolKit.h"

#include <random>

#include "DebugNew.h"

namespace ToolKit
{

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

    m_ssaoShader->SetShaderParameter("radius",
                                     ParameterVariant(m_params.ssaoRadius));
    m_ssaoShader->SetShaderParameter("bias",
                                     ParameterVariant(m_params.ssaoBias));
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
        float u1    = randomFloats(generator);
        float u2    = randomFloats(generator);
        float r     = glm::sqrt(u1);
        float theta = 2.0f * glm::pi<float>() * u2;
        float x     = r * cos(theta);
        float y     = r * sin(theta);
        float z     = glm::sqrt(glm::max(0.0f, 1.0f - x * x - y * y));

        Vec3 sample(x, y, z);

        /* sample = glm::normalize(sample);
        sample      *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale       = lerp(0.1f, 1.0f, scale * scale);
        sample      *= scale;*/
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