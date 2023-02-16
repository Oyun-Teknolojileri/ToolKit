#pragma once

#include "BinPack2D.h"
#include "DataTexture.h"
#include "Framebuffer.h"
#include "GeometryTypes.h"
#include "Primative.h"
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
    Mesh* Mesh                       = nullptr;
    SkeletonComponentPtr SkeletonCmp = nullptr;
    MaterialPtr Material             = nullptr;
    bool ShadowCaster                = true;
    BoundingBox BoundingBox;
    Mat4 WorldTransform;
  };

  class TK_API RenderJobProcessor
  {
   public:
    static void CreateRenderJobs(EntityRawPtrArray entities,
                                 RenderJobArray& jobArray);

    static void CreateRenderJob(Entity* entity, RenderJob& job);

    static void SeperateDeferredForward(const RenderJobArray& jobArray,
                                        RenderJobArray& deferred,
                                        RenderJobArray& forward,
                                        RenderJobArray& translucent);

    /**
     * Utility function that sorts lights according to lit conditions from
     * best to worst. Make sure lights array has updated shadow camera. Shadow
     * camera is used in culling calculations.
     */
    static LightRawPtrArray SortLights(const RenderJob& job,
                                       const LightRawPtrArray& lights);

    static LightRawPtrArray SortLights(Entity* entity,
                                       const LightRawPtrArray& lights);

    // Sort entities  by distance (from boundary center)
    // in ascending order to camera. Accounts for isometric camera.
    static void StableSortByDistanceToCamera(RenderJobArray& entities,
                                             const Camera* cam);

    static void CullRenderJobs(RenderJobArray& jobArray, Camera* camera);
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
