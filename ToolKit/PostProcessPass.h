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
    ShaderPtr m_postProcessShader;
    FullQuadPassPtr m_postProcessPass = nullptr;
    FramebufferPtr m_copyBuffer       = nullptr;
    RenderTargetPtr m_copyTexture     = nullptr;
  };

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

} // namespace ToolKit