/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
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
    FramebufferPtr m_prevFrameBuffer = nullptr;

   private:
    Renderer* m_renderer = nullptr;
  };

  /**
   * This struct holds all the data required to make a drawcall.
   */
  struct RenderJob
  {
    EntityPtr Entity                          = nullptr; //!< Entity that this job is created from.
    Mesh* Mesh                                = nullptr; //!< Mesh to render.
    MaterialPtr Material                      = nullptr; //!< Material to render job with.
    EnvironmentComponentPtr EnvironmentVolume = nullptr; //!< EnvironmentVolume effecting this entity, if any.
    bool ShadowCaster                         = true;    //!< Account in shadow map construction.
    bool frustumCulled                        = false;
    BoundingBox BoundingBox; //!< World space bounding box.
    Mat4 WorldTransform;     //!< World transform of the entity.
    AnimData animData;       //!< Animation data of render job
  };

  typedef RenderJobArray::iterator RenderJobItr;

  /**
   * Singular render data that contains all the rendering information for a frame.
   */
  struct RenderData
  {
    RenderJobArray jobs;
    const int deferredJobsStartIndex = 0; // Job array always start with deferred.
    int forwardOpaqueStartIndex      = 0;
    int forwardTranslucentStartIndex = 0;

    RenderJobItr GetForwardOpaqueBegin() { return jobs.begin() + forwardOpaqueStartIndex; }

    RenderJobItr GetForwardTranslucentBegin() { return GetForwardOpaqueBegin() + forwardTranslucentStartIndex; }
  };

  class TK_API RenderJobProcessor
  {
   public:
    /**
     * Constructs all render jobs from entities. Also constructs the bounding volume that covers all entities.
     * Good candidate to use as shadow boundary.
     * @param entities are the entities to construct render jobs for.
     * @param jobArray is the array of constructed jobs.
     * @param ingnoreVisibility when set true, construct jobs for objects that has visibility set to false.
     * @return BoundingBox for jobs.
     */
    static BoundingBox CreateRenderJobs(const EntityPtrArray& entities,
                                        RenderJobArray& jobArray,
                                        bool ignoreVisibility = false);

    static void SeperateDeferredForward(const RenderJobArray& jobArray,
                                        RenderJobArray& deferred,
                                        RenderJobArray& forward,
                                        RenderJobArray& translucent);

    static void SeperateRenderData(RenderData& renderData);

    static void SeperateOpaqueTranslucent(const RenderJobArray& jobArray,
                                          RenderJobArray& opaque,
                                          RenderJobArray& translucent);

    /**
     * Utility function that sorts lights according to lit conditions from
     * best to worst. Make sure lights array has updated shadow camera. Shadow
     * camera is used in culling calculations.
     * @returns Number of lights effecting the job. From index 0 to return number contains effective lights in the
     * sorted array.
     */
    static int SortLights(const RenderJob& job, LightPtrArray& lights, int startFromIndex = 0);

    /**
     * Makes sure that first elements are directional lights.
     * @param lights are the lights to sort.
     * @returns The index where the non directional lights starts.
     */
    static int PreSortLights(LightPtrArray& lights);

    static LightPtrArray SortLights(EntityPtr entity, LightPtrArray& lights);

    // Sort entities  by distance (from boundary center)
    // in ascending order to camera. Accounts for isometric camera.
    static void SortByDistanceToCamera(RenderJobItr begin, RenderJobItr end, const CameraPtr& cam);

    /**
     * Cull objects based on the sent camera. Update Job's frustumCulled state.
     */
    static void CullRenderJobs(RenderJobArray& jobArray, const CameraPtr& camera);

    /**
     * Doesn't alter render job, but puts results into cullResults array. Useful for not to alter RenderData.
     */
    static void CullRenderJobs(const RenderJobArray& jobArray, const CameraPtr& camera, BoolArray& results);

    static void StableSortByMeshThanMaterail(RenderData& renderData);

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
