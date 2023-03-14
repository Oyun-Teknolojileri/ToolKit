#pragma once

#include "PostProcessPass.h"

namespace ToolKit
{

  enum class DoFQuality
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