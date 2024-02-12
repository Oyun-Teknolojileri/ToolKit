/*
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
    GpuProgramPtr m_program          = nullptr;

   private:
    Renderer* m_renderer = nullptr;
  };

  /**
   * This struct holds all the data required to make a drawcall.
   */
  struct RenderJob
  {
    Entity* Entity                          = nullptr; //!< Entity that this job is created from.
    Mesh* Mesh                              = nullptr; //!< Mesh to render.
    Material* Material                      = nullptr; //!< Material to render job with.
    EnvironmentComponent* EnvironmentVolume = nullptr; //!< EnvironmentVolume effecting this entity, if any.
    bool ShadowCaster                       = true;    //!< Account in shadow map construction.
    bool frustumCulled                      = false;   //!< States that the job is culled by a camera.

    BoundingBox BoundingBox; //!< World space bounding box.
    Mat4 WorldTransform;     //!< World transform of the entity.
    AnimData animData;       //!< Animation data of render job.

    short activeLightCount = 0; //!< Number of lights that is active in the list.
    std::array<int, Renderer::RHIConstants::MaxLightsPerObject> lights {}; //!< Light indexes that affects the job.
  };

  typedef RenderJobArray::iterator RenderJobItr;

  /**
   * Singular render data that contains all the rendering information for a frame.
   * When first culled than separated by a render job processor, the indexes become valid.
   * Partition structure
   * 0 Culled               : jobs.begin to deferredJobsStartIndex
   * 1 Deferred             : deferredJobsStartIndex to forwardOpaqueStartIndex
   * 2 Forward Opaque       : forwardOpaqueStartIndex to forwardTranslucentStartIndex
   * 3 Forward Translucent  : forwardTranslucentStartIndex to jobs.end
   */
  struct RenderData
  {
    RenderJobArray jobs;

    int deferredJobsStartIndex       = 0; //!< Beginning of deferred jobs. Before this, culled jobs resides.
    int forwardOpaqueStartIndex      = 0; //!< Beginning of forward opaque jobs.
    int forwardTranslucentStartIndex = 0; //!< Beginning of forward translucent jobs.

    RenderJobItr GetDefferedBegin()
    {
      assert(deferredJobsStartIndex != -1 && "Accessing forward only data.");
      return jobs.begin() + deferredJobsStartIndex;
    }

    RenderJobItr GetForwardOpaqueBegin() { return jobs.begin() + forwardOpaqueStartIndex; }

    RenderJobItr GetForwardTranslucentBegin() { return jobs.begin() + forwardTranslucentStartIndex; }
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
    static void CreateRenderJobs(const EntityPtrArray& entities,
                                 RenderJobArray& jobArray,
                                 bool ignoreVisibility = false);

    static void CreateRenderJobs(const EntityPtrArray& entities,
                                 RenderJobArray& jobArray,
                                 LightPtrArray& lights,
                                 const CameraPtr& camera,
                                 const EnvironmentComponentPtrArray& environments,
                                 bool ignoreVisibility = false);

    /**
     * This will drop the lights whose bounding volume does not intersect with camera.
     */
    static void CullLights(LightPtrArray& lights, const CameraPtr& camera, float maxDistance = TK_FLT_MAX);

    /**
     * Separate jobs such that job array starts with culled jobs, than deferred jobs, than forward opaque and
     * translucent jobs.
     * For example, all jobs between these iterators are the deferred jobs.
     * RenderData::GetDefferedBegin() and RenderData::GetForwardOpaqueBegin()
     */
    static void SeperateRenderData(RenderData& renderData, bool forwardOnly);

    static void AssignLight(RenderJob& job, LightPtrArray& lights, int startIndex);

    /**
     * Assign all lights affecting the job.
     */
    static void AssignLight(RenderJobItr begin, RenderJobItr end, LightPtrArray& lights);

    /**
     * Makes sure that first elements are directional lights.
     * @param lights are the lights to sort.
     * @returns The index where the non directional lights starts.
     */
    static int PreSortLights(LightPtrArray& lights);

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
    static void CullRenderJobs(const RenderJobArray& jobArray, const CameraPtr& camera, UIntArray& resultIndices);

    static void CullRenderJobs(const RenderJobArray& jobArray, const CameraPtr& camera, RenderJobArray& unCulledJobs);

    static void StableSortByMeshThanMaterail(RenderData& renderData);

    static void AssignEnvironment(RenderJobItr begin,
                                  RenderJobItr end,
                                  const EnvironmentComponentPtrArray& environments);

    static void AssignEnvironment(RenderJob& job, const EnvironmentComponentPtrArray& environments);

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
