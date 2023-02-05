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
    Mesh* Mesh           = nullptr;
    MaterialPtr Material = nullptr;
    Mat4 WorldTransform;
  };

  typedef std::vector<RenderJob> RenderJobArray;

  class RenderJobProcessor
  {
   public:
    static void CreateRenderJobs(EntityRawPtrArray entities,
                                 RenderJobArray& jobArray);

    static void SeperateDeferredForward(const RenderJobArray& jobArray,
                                        RenderJobArray& deferred,
                                        RenderJobArray& forward,
                                        RenderJobArray& translucent);
  };

  /*
   * Base class for main rendering classes.
   */
  class TK_API RenderPass : public Pass
  {
   public:
    RenderPass();
    virtual ~RenderPass();

    // Sort entities  by distance (from boundary center)
    // in ascending order to camera. Accounts for isometric camera.
    void StableSortByDistanceToCamera(EntityRawPtrArray& entities,
                                      const Camera* cam);

    /**
     * Extracts translucent entities from given entity array.
     * @param entities Entity array that the translucent will extracted from.
     * @param translucent Entity array that contains translucent entities.
     */
    void SeperateTranslucentEntities(const EntityRawPtrArray& allEntities,
                                     EntityRawPtrArray& opaqueEntities,
                                     EntityRawPtrArray& translucentEntities);

    /**
     * Extracts translucent and unlit entities from given entity array.
     * @param entities Entity array that the translucent will extracted from.
     * @param translucentAndUnlit Entity array that contains translucent and
     * unlit entities.
     */
    void SeperateTranslucentAndUnlitEntities(
        const EntityRawPtrArray& allEntities,
        EntityRawPtrArray& opaqueEntities,
        EntityRawPtrArray& translucentAndUnlitEntities);

    void CreateRenderJobs(EntityRawPtrArray entities);

   protected:
    RenderJobArray m_renderJobs;
  };

} // namespace ToolKit
