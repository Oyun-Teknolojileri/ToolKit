/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "Pass.h"

namespace ToolKit
{

  struct ForwardRenderPassParams
  {
    CameraPtr Cam                  = nullptr;
    FramebufferPtr FrameBuffer     = nullptr;
    FramebufferPtr gFrameBuffer    = nullptr;
    RenderTargetPtr gNormalRt      = nullptr;
    RenderTargetPtr gLinearRt      = nullptr;
    bool ClearFrameBuffer          = true;  //!< Clears whole buffer
    bool ClearDepthBuffer          = false; //!< Clears only depth buffer.
    bool SSAOEnabled               = false;
    RenderJobArray OpaqueJobs      = {};
    RenderJobArray TranslucentJobs = {};
    LightPtrArray Lights           = {};
  };

  /**
   * Renders given entities with given lights using forward rendering
   */
  class TK_API ForwardRenderPass : public RenderPass
  {
   public:
    ForwardRenderPass();
    explicit ForwardRenderPass(const ForwardRenderPassParams& params);
    ~ForwardRenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   protected:
    /**
     * Renders the entities immediately. No sorting applied.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param zoom Zoom amount of camera.
     * @param lights All lights.
     */
    void RenderOpaque(RenderJobArray& jobs, CameraPtr cam, const LightPtrArray& lights);

    /**
     * Sorts and renders translucent entities. For double-sided blended entities
     * first render back, than renders front.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param lights ights All lights.
     */
    void RenderTranslucent(RenderJobArray& jobs, CameraPtr cam, const LightPtrArray& lights);

   public:
    ForwardRenderPassParams m_params;
  };

  typedef std::shared_ptr<ForwardRenderPass> ForwardRenderPassPtr;
} // namespace ToolKit