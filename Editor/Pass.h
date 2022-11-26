#pragma once
#pragma once

#include "FrameBuffer.h"
#include "GeometryTypes.h"
#include "Primative.h"

namespace ToolKit
{

  /**
   * Base Pass class.
   */
  class Pass
  {
   public:
    Pass();
    virtual ~Pass();

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
    bool ClearFrameBuffer      = true;
  };

  /**
   * Renders the given scene with full forward render pipeline.
   */
  class RenderPass : public Pass
  {
   public:
    RenderPass();
    explicit RenderPass(const RenderPassParams& params);
    virtual ~RenderPass();

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
    Camera* m_camera = nullptr;
    EntityRawPtrArray m_drawList;
    LightRawPtrArray m_contributingLights;
  };

  typedef std::shared_ptr<RenderPass> RenderPassPtr;

  struct ShadowPassParams
  {
    LightRawPtrArray Lights;
    EntityRawPtrArray Entities;
  };

  /**
   * Create shadow map buffers for all given lights.
   */
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

  typedef std::shared_ptr<ShadowPass> ShadowPassPtr;

  struct FullQuadPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    ShaderPtr FragmentShader   = nullptr;
    bool ClearFrameBuffer      = true;
  };

  /**
   * Draws a full quad that covers entire FrameBuffer with given fragment
   * shader.
   */
  class FullQuadPass : public Pass
  {
   public:
    FullQuadPass();
    explicit FullQuadPass(const FullQuadPassParams& params);
    ~FullQuadPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    FullQuadPassParams m_params;

   private:
    CameraPtr m_camera;
    QuadPtr m_quad;
    MaterialPtr m_material;
  };

  typedef std::shared_ptr<FullQuadPass> FullQuadPassPtr;

  struct StencilRenderPassParams
  {
    RenderTargetPtr OutputTarget;
    EntityRawPtrArray DrawList;
    Camera* Camera = nullptr;
  };

  /**
   * Creates a binary stencil buffer from the given entities and copies the
   * binary image to OutputTarget.
   */
  class StencilRenderPass : public Pass
  {
   public:
    StencilRenderPass();
    explicit StencilRenderPass(const StencilRenderPassParams& params);

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    StencilRenderPassParams m_params;

   private:
    FramebufferPtr m_frameBuffer         = nullptr;
    MaterialPtr m_solidOverrideMaterial  = nullptr;
    FullQuadPassPtr m_copyStencilSubPass = nullptr;
  };

  typedef std::shared_ptr<StencilRenderPass> StencilRenderPassPtr;

  struct OutlinePassParams
  {
    EntityRawPtrArray DrawList;
    FramebufferPtr FrameBuffer = nullptr;
    Camera* Camera             = nullptr;
    Vec4 OutlineColor          = Vec4(1.0f);
  };

  /**
   * Draws given entities' outlines to the FrameBuffer.
   */
  class OutlinePass : public Pass
  {
   public:
    OutlinePass();
    explicit OutlinePass(const OutlinePassParams& params);

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    OutlinePassParams m_params;

   private:
    StencilRenderPassPtr m_stencilPass = nullptr;
    FullQuadPassPtr m_outlinePass      = nullptr;
    ShaderPtr m_dilateShader           = nullptr;
    RenderTargetPtr m_stencilAsRt      = nullptr;
  };

  typedef std::shared_ptr<OutlinePass> OutlinePassPtr;

  struct GammaPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    float Gamma                = 2.2f;
  };

  /**
   * Apply gamma correction to given frame buffer.
   */
  class GammaPass : public Pass
  {
   public:
    GammaPass();
    explicit GammaPass(const GammaPassParams& params);

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    GammaPassParams m_params;

   private:
    FullQuadPassPtr m_gammaPass   = nullptr;
    FramebufferPtr m_copyBuffer   = nullptr;
    RenderTargetPtr m_copyTexture = nullptr;
    ShaderPtr m_gammaShader       = nullptr;
  };

} // namespace ToolKit