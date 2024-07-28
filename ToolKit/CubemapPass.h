/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct CubeMapPassParams
  {
    FramebufferPtr FrameBuffer   = nullptr;
    CameraPtr Cam                = nullptr;
    MaterialPtr Material         = nullptr;
    CompareFunctions DepthFn     = CompareFunctions::FuncLequal;
    GraphicBitFields clearBuffer = GraphicBitFields::AllBits;
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