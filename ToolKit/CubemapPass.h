#pragma once
#include "Pass.h"

namespace ToolKit
{

  struct CubeMapPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    Camera* Cam                = nullptr;
    MaterialPtr Material       = nullptr;
    CompareFunctions DepthFn   = CompareFunctions::FuncLequal;
    Mat4 Transform;
  };

  class TK_API CubeMapPass : public Pass
  {
   public:
    CubeMapPass();
    explicit CubeMapPass(const CubeMapPassParams& params);
    ~CubeMapPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    CubeMapPassParams m_params;

   private:
    CubePtr m_cube = nullptr;
  };

  typedef std::shared_ptr<CubeMapPass> CubeMapPassPtr;

} // namespace ToolKit