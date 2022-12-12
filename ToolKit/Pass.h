#pragma once

#include "BinPack2D.h"
#include "Framebuffer.h"
#include "GeometryTypes.h"
#include "Primative.h"

namespace ToolKit
{

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

   protected:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;
  };

  struct RenderPassParams
  {
    EntityRawPtrArray Entities;
    LightRawPtrArray Lights;
    Camera* Cam                = nullptr;
    FramebufferPtr FrameBuffer = nullptr;
    bool ClearFrameBuffer      = true;
  };

  /**
   * Renders given entities with given lights using forward rendering
   */
  class TK_API RenderPass : public Pass
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
     * @param lights ights All lights.
     */
    void RenderTranslucent(EntityRawPtrArray entities,
                           Camera* cam,
                           const LightRawPtrArray& lights);

   public:
    RenderPassParams m_params;

   protected:
    Camera* m_camera = nullptr;
    EntityRawPtrArray m_drawList;
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
  class TK_API ShadowPass : public Pass
  {
   public:
    ShadowPass();
    explicit ShadowPass(const ShadowPassParams& params);
    ~ShadowPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

    RenderTargetPtr GetShadowAtlas();

   private:
    void RenderShadowMaps(Light* light, const EntityRawPtrArray& entities);
    void FilterShadowMap(Light* light);

    /**
     * Sets layer and coordintes of the shadow maps in shadow atlas.
     * @param lights Light array that have shadows.
     * @return number of layers needed.
     */
    int PlaceShadowMapsToShadowAtlas(const LightRawPtrArray& lights);

    /**
     * Creates a shadow atlas for m_params.Lights
     */
    void InitShadowAtlas();

   public:
    ShadowPassParams m_params;

   private:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;
    MaterialPtr m_lastOverrideMat      = nullptr;

    FramebufferPtr m_shadowFramebuffer = nullptr;
    RenderTargetPtr m_shadowAtlas      = nullptr;
    int m_layerCount = 0; // Number of textures in array texture (shadow atlas)
    EntityIdArray m_previousShadowCasters;
    std::vector<bool> m_clearedLayers;

    EntityRawPtrArray m_drawList;
    Quaternion m_cubeMapRotations[6];
    Vec3 m_cubeMapScales[6];

    BinPack2D m_packer;
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
  class TK_API FullQuadPass : public Pass
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
  class TK_API StencilRenderPass : public Pass
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
   * TODO: It should be Technique instead of Pass
   */
  class TK_API OutlinePass : public Pass
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
  class TK_API GammaPass : public Pass
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

  struct SceneRenderPassParams
  {
    ScenePtr Scene = nullptr;
    LightRawPtrArray Lights;
    Camera* Cam                    = nullptr;
    FramebufferPtr MainFramebuffer = nullptr;
    bool ClearFramebuffer          = true;
  };

  struct GBufferPassParams
  {
    EntityRawPtrArray entities;
    Camera* camera;
  };

  class TK_API GBufferPass : public Pass
  {
   public:
    GBufferPass();

    void PreRender() override;
    void PostRender() override;
    void Render() override;
    void InitGBuffers(int width, int height);
    void UnInitGBuffers();

   public:
    FramebufferPtr m_framebuffer = nullptr;
    RenderTargetPtr m_gPosRt     = nullptr;
    RenderTargetPtr m_gNormalRt  = nullptr;
    RenderTargetPtr m_gColorRt   = nullptr;

    int m_width  = 1024;
    int m_height = 1024;

    GBufferPassParams m_params;

   private:
    bool m_initialized = false;
    bool m_attachmentsSet         = false;
    MaterialPtr m_gBufferMaterial = nullptr;
  };

  /**
   * Render scene with shadows.
   * TODO: It should be Tecnhique instead of Pass.
   */
  class TK_API SceneRenderPass : public Pass
  {
   public:
    SceneRenderPass();
    explicit SceneRenderPass(const SceneRenderPassParams& params);

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   private:
    void SetPassParams();

   public:
    SceneRenderPassParams m_params;

    ShadowPassPtr m_shadowPass = nullptr;
    RenderPassPtr m_renderPass = nullptr;
    GBufferPass m_gBufferPass;
  };

} // namespace ToolKit
