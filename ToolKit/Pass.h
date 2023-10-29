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

#include "EnvironmentComponent.h"
#include "Renderer.h"

namespace ToolKit
{

  typedef std::shared_ptr<class Pass> PassPtr;
  typedef std::vector<PassPtr> PassPtrArray;

  /**
   * Base Pass class.
   */
  class TK_API Pass
  {
   public:
    Pass();
    virtual ~Pass();

    virtual void Render() = 0;
    virtual void PreRender();
    virtual void PostRender();
    void RenderSubPass(const PassPtr& pass);

    Renderer* GetRenderer();
    void SetRenderer(Renderer* renderer);

   protected:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;

   private:
    Renderer* m_renderer = nullptr;
  };

  struct RenderJob
  {
    EntityPtr Entity                          = nullptr;
    Mesh* Mesh                                = nullptr;
    SkeletonComponentPtr SkeletonCmp          = nullptr;
    MaterialPtr Material                      = nullptr;
    EnvironmentComponentPtr EnvironmentVolume = nullptr;
    bool ShadowCaster                         = true;
    BoundingBox BoundingBox;
    Mat4 WorldTransform;
  };

  class TK_API RenderJobProcessor
  {
   public:
    static void CreateRenderJobs(const EntityPtrArray& entities,
                                 RenderJobArray& jobArray,
                                 bool ignoreVisibility = false);

    static void SeperateDeferredForward(const RenderJobArray& jobArray,
                                        RenderJobArray& deferred,
                                        RenderJobArray& forward,
                                        RenderJobArray& translucent);

    static void SeperateOpaqueTranslucent(const RenderJobArray& jobArray,
                                          RenderJobArray& opaque,
                                          RenderJobArray& translucent);

    /**
     * Utility function that sorts lights according to lit conditions from
     * best to worst. Make sure lights array has updated shadow camera. Shadow
     * camera is used in culling calculations.
     */
    static void SortLights(const RenderJob& job, LightPtrArray& lights);

    static LightPtrArray SortLights(EntityPtr entity, LightPtrArray& lights);

    // Sort entities  by distance (from boundary center)
    // in ascending order to camera. Accounts for isometric camera.
    static void SortByDistanceToCamera(RenderJobArray& jobArray, const CameraPtr cam);

    static void CullRenderJobs(RenderJobArray& jobArray, CameraPtr camera);

    static void AssignEnvironment(RenderJobArray& jobArray, const EnvironmentComponentPtrArray& environments);

    /**
     * Calculates the standard deviation and mean of the given RenderJobArray
     * based on world position of the RenderJobs.
     * @param rjVec is array containing jobs.
     * @param stdev is the output of calculated standard deviation.
     * @param mean is the calculated mean position.
     */
    static void CalculateStdev(const RenderJobArray& rjVec, float& stdev, Vec3& mean);

    /**
     * Decides if the given RenderJob is an outlier based on its world position.
     * @param rj is the RenderJob to decide if its outlier.
     * @param sigma is the threshold sigma to accept as outlier or not.
     */
    static bool IsOutlier(const RenderJob& rj, float sigma, const float stdev, const Vec3& mean);
  };

  /*
   * Base class for main rendering classes.
   */
  class TK_API RenderPass : public Pass
  {
   public:
    RenderPass();
    virtual ~RenderPass();
  };

} // namespace ToolKit
