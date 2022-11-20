#pragma once

#include "FrameBuffer.h"
#include "GeometryTypes.h"

namespace ToolKit
{

  class Pass
  {
   public:
    virtual void Render() = 0;
    virtual void PreRender();
    virtual void PostRender();

   protected:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;
  };

  struct RenderPassParams
  {
    ScenePtr Scene;
    LightRawPtrArray LightOverride;
    Camera* Cam                = nullptr;
    FramebufferPtr FrameBuffer = nullptr;
    float BillboardScale       = 1.0f;
  };

  class RenderPass : public Pass
  {
   public:
    RenderPass();
    explicit RenderPass(const RenderPassParams& params);
    ~RenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   protected:
    void CullDrawList(EntityRawPtrArray& entities, Camera* camera);
    void CullLightList(Entity const* entity, LightRawPtrArray& lights);

    /**
     * Extracts translucent entities from given entity array.
     * @param entities Entity array that the translucent will extracted from.
     * @param translucent Entity array that contains translucent entities.
     */
    void SeperateTranslucentEntities(EntityRawPtrArray& entities,
                                     EntityRawPtrArray& translucentEntities);

    /**
     * Renders the entities immediately. No sorting applied.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param zoom Zoom amount of camera.
     * @param lights All lights.
     */
    void RenderOpaque(EntityRawPtrArray entities,
                      Camera* cam,
                      const LightRawPtrArray& lights);

    /**
     * Sorts and renders translucent entities. For double-sided blended entities
     * first render back, than renders front.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param editorLights All lights.
     */
    void RenderTranslucent(EntityRawPtrArray entities,
                           Camera* cam,
                           const LightRawPtrArray& lights);

   public:
    RenderPassParams m_params;

   protected:
    EntityRawPtrArray m_drawList;
    LightRawPtrArray m_contributingLights;
  };

  struct ShadowPassParams
  {
    LightRawPtrArray Lights;
    EntityRawPtrArray Entities;
  };

  class ShadowPass : public Pass
  {
   public:
    ShadowPass();
    explicit ShadowPass(const ShadowPassParams& params);
    ~ShadowPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   private:
    void UpdateShadowMap(Light* light, const EntityRawPtrArray& entities);
    void FilterShadowMap(Light* light);

   public:
    ShadowPassParams m_params;

   private:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;

    EntityRawPtrArray m_drawList;
    Quaternion m_cubeMapRotations[6];
    Vec3 m_cubeMapScales[6];
  };

} // namespace ToolKit