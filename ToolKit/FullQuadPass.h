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

  struct FullQuadPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    BlendFunction BlendFunc    = BlendFunction::NONE;
    bool ClearFrameBuffer      = true;
  };

  /**
   * Draws a full quad that covers entire FrameBuffer with given fragment
   * shader.
   */
  class TK_API FullQuadPass : public Pass
  {
   public:
    FullQuadPass();
    explicit FullQuadPass(const FullQuadPassParams& params);
    ~FullQuadPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

    /**
     * This function should be called in order to create material and program of quad render
     */
    void SetFragmentShader(ShaderPtr fragmentShader, Renderer* renderer);

    /**
     * This function is used to pass custom uniforms to this pass
     */
    void UpdateUniform(const ShaderUniform& shaderUniform);

   public:
    FullQuadPassParams m_params;
    MaterialPtr m_material = nullptr;

   private:
    CameraPtr m_camera;
    QuadPtr m_quad;
  };

  typedef std::shared_ptr<FullQuadPass> FullQuadPassPtr;

} // namespace ToolKit