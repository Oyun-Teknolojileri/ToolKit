#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct BloomPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    int iterationCount         = 6;
    float minThreshold = 1.0f, intensity = 1.0f;
  };

  class TK_API BloomPass : public Pass
  {
   public:
    BloomPass();
    explicit BloomPass(const BloomPassParams& params);
    ~BloomPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    BloomPassParams m_params;

   private:
    // Iteration Count + 1 number of textures & framebuffers
    std::vector<RenderTargetPtr> m_tempTextures;
    std::vector<FramebufferPtr> m_tempFrameBuffers;
    FullQuadPassPtr m_pass       = nullptr;
    ShaderPtr m_downsampleShader = nullptr;
    ShaderPtr m_upsampleShader   = nullptr;

    bool m_invalidRenderParams   = false;
  };

  typedef std::shared_ptr<BloomPass> BloomPassPtr;

  struct PostProcessPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    ShaderPtr Shader           = nullptr;
  };

  class TK_API PostProcessPass : public Pass
  {
   public:
    PostProcessPass();
    explicit PostProcessPass(const PostProcessPassParams& params);
    ~PostProcessPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    PostProcessPassParams m_params;

   protected:
    ShaderPtr m_postProcessShader     = nullptr;
    FullQuadPassPtr m_postProcessPass = nullptr;
    FramebufferPtr m_copyBuffer       = nullptr;
    RenderTargetPtr m_copyTexture     = nullptr;
  };

  struct FXAAPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    Vec2 screen_size;
  };

  class TK_API FXAAPass : public PostProcessPass
  {
   public:
    FXAAPass();
    explicit FXAAPass(const FXAAPassParams& params);

    void PreRender() override;

   public:
    FXAAPassParams m_params;
  };

  typedef std::shared_ptr<FXAAPass> FXAAPassPtr;

  struct GammaPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    float Gamma                = 2.2f;
  };

  /**
   * Apply gamma correction to given frame buffer.
   */
  class TK_API GammaPass : public PostProcessPass
  {
   public:
    GammaPass();
    explicit GammaPass(const GammaPassParams& params);
    ~GammaPass();

    void PreRender() override;

   public:
    GammaPassParams m_params;
  };

  typedef std::shared_ptr<GammaPass> GammaPassPtr;

  enum class TonemapMethod
  {
    Reinhard,
    Aces
  };

  struct TonemapPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    TonemapMethod Method       = TonemapMethod::Aces;
  };

  class TK_API TonemapPass : public PostProcessPass
  {
   public:
    TonemapPass();
    explicit TonemapPass(const TonemapPassParams& params);
    ~TonemapPass();

    void PreRender() override;

   public:
    TonemapPassParams m_params;
  };

  typedef std::shared_ptr<TonemapPass> TonemapPassPtr;

  struct SSAOPassParams
  {
    TexturePtr GPositionBuffer    = nullptr;
    TexturePtr GNormalBuffer      = nullptr;
    TexturePtr GLinearDepthBuffer = nullptr;
    Camera* Cam                   = nullptr;
    float ssaoRadius              = 0.5f;
    float ssaoBias                = 0.025f;
  };

  class TK_API SSAOPass : public Pass
  {
   public:
    SSAOPass();
    explicit SSAOPass(const SSAOPassParams& params);
    ~SSAOPass();

    void Render();
    void PreRender();
    void PostRender();

   private:
    void GenerateSSAONoise();

   public:
    SSAOPassParams m_params;
    RenderTargetPtr m_ssaoTexture = nullptr;

   private:
    Vec3Array m_ssaoKernel;
    Vec2Array m_ssaoNoise;

    FramebufferPtr m_ssaoFramebuffer   = nullptr;
    SSAONoiseTexturePtr m_noiseTexture = nullptr;
    RenderTargetPtr m_tempBlurRt       = nullptr;

    FullQuadPassPtr m_quadPass         = nullptr;
    ShaderPtr m_ssaoShader             = nullptr;
  };

  typedef std::shared_ptr<SSAOPass> SSAOPassPtr;

  enum class DoFQuality : uint
  {
    Low,    // Radius Scale = 2.0f
    Normal, // Radius Scale = 0.8f
    High    // Radius Scale = 0.2f
  };

  struct DoFPassParams
  {
    RenderTargetPtr ColorRt = nullptr;
    RenderTargetPtr DepthRt = nullptr;
    float focusPoint        = 0.0f;
    float focusScale        = 0.0f;
    DoFQuality blurQuality  = DoFQuality::Normal;
  };

  class TK_API DoFPass : public Pass
  {
   public:
    DoFPass();
    explicit DoFPass(const DoFPassParams& params);
    ~DoFPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    DoFPassParams m_params;

   private:
    FullQuadPassPtr m_quadPass = nullptr;
    ShaderPtr m_dofShader      = nullptr;
  };

  typedef std::shared_ptr<DoFPass> DoFPassPtr;
} // namespace ToolKit