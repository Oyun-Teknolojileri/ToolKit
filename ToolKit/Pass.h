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

  /*
   * Base class for main rendering classes.
   */
  class TK_API RenderPass : public Pass
  {
   public:
    // Sort entities  by distance (from boundary center)
    // in ascending order to camera. Accounts for isometric camera.
    void StableSortByDistanceToCamera(EntityRawPtrArray& entities,
                                      const Camera* cam);

    // Sort entities by their material's render state's priority in
    // descending order.
    void StableSortByMaterialPriority(EntityRawPtrArray& entities);

    /**
     * Extracts translucent entities from given entity array.
     * @param entities Entity array that the translucent will extracted from.
     * @param translucent Entity array that contains translucent entities.
     */
    void SeperateTranslucentEntities(EntityRawPtrArray& entities,
                                     EntityRawPtrArray& translucentEntities);

    /**
     * Extracts translucent and unlit entities from given entity array.
     * @param entities Entity array that the translucent will extracted from.
     * @param translucentAndUnlit Entity array that contains translucent and
     * unlit entities.
     */
    void SeperateTranslucentAndUnlitEntities(
        EntityRawPtrArray& entities,
        EntityRawPtrArray& translucentAndUnlitEntities);
  };

  struct ForwardRenderPassParams
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
  class TK_API ForwardRenderPass : public RenderPass
  {
   public:
    ForwardRenderPass();
    explicit ForwardRenderPass(const ForwardRenderPassParams& params);
    virtual ~ForwardRenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   protected:
    void CullLightList(Entity const* entity, LightRawPtrArray& lights);

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
    ForwardRenderPassParams m_params;

   protected:
    Camera* m_camera = nullptr;
    EntityRawPtrArray m_drawList;
  };

  typedef std::shared_ptr<ForwardRenderPass> ForwardRenderPassPtr;

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
    BlendFunction BlendFunc    = BlendFunction::NONE;
    ShaderPtr FragmentShader   = nullptr;
    bool ClearFrameBuffer      = true;
    LightRawPtrArray lights;
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

  struct BloomPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    int iterationCount         = 6;
    float minThreshold = 1.0f, intensity = 1.0f;
  };

  class TK_API BloomPass : public Pass
  {
   public:
    BloomPass();
    explicit BloomPass(const BloomPassParams& params);

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    BloomPassParams m_params;

   private:
    // Iteration Count + 1 number of textures & framebuffers
    std::vector<RenderTargetPtr> m_tempTextures;
    std::vector<FramebufferPtr> m_tempFrameBuffers;
    FullQuadPassPtr m_pass       = nullptr;
    ShaderPtr m_downsampleShader = nullptr;
    ShaderPtr m_upsampleShader   = nullptr;

    bool m_invalidRenderParams = false;
  };

  struct PostProcessPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    ShaderPtr Shader           = nullptr;
  };

  class TK_API PostProcessPass : public Pass
  {
   public:
    PostProcessPass();
    explicit PostProcessPass(const PostProcessPassParams& params);

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    PostProcessPassParams m_params;

   protected:
    ShaderPtr m_postProcessShader;
    FullQuadPassPtr m_postProcessPass = nullptr;
    FramebufferPtr m_copyBuffer       = nullptr;
    RenderTargetPtr m_copyTexture     = nullptr;
  };

  struct GammaPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    float Gamma                = 2.2f;
  };

  /**
   * Apply gamma correction to given frame buffer.
   */
  class TK_API GammaPass : public PostProcessPass
  {
   public:
    GammaPass();
    explicit GammaPass(const GammaPassParams& params);

    void PreRender() override;

   public:
    GammaPassParams m_params;
  };

  struct TonemapPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    enum TonemapMethod
    {
      Reinhard,
      Aces
    };
    TonemapMethod Method = Aces;
  };

  class TK_API TonemapPass : public PostProcessPass
  {
   public:
    TonemapPass();
    explicit TonemapPass(const TonemapPassParams& params);

    void PreRender() override;

   public:
    TonemapPassParams m_params;
  };

  typedef std::shared_ptr<TonemapPass> TonemapPassPtr;

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
    bool m_initialized            = false;
    bool m_attachmentsSet         = false;
    MaterialPtr m_gBufferMaterial = nullptr;
  };

  struct DeferredRenderPassParams
  {
    FramebufferPtr MainFramebuffer;
    FramebufferPtr GBufferFramebuffer;
    bool ClearFramebuffer = true;
    LightRawPtrArray lights;
    Camera* GBufferCamera = nullptr;
  };

  class TK_API DeferredRenderPass : public RenderPass
  {
   public:
    DeferredRenderPass();
    DeferredRenderPass(const DeferredRenderPassParams& params);

    void PreRender() override;
    void PostRender() override;
    void Render() override;

   public:
    DeferredRenderPassParams m_params;

   private:
    FullQuadPass m_fullQuadPass;
    ShaderPtr m_deferredRenderShader = nullptr;
  };

  struct SceneRenderPassParams
  {
    ScenePtr Scene = nullptr;
    LightRawPtrArray Lights;
    Camera* Cam                    = nullptr;
    FramebufferPtr MainFramebuffer = nullptr;
    bool ClearFramebuffer          = true;
  };

  /**
   * Main scene renderer.
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

    void CullDrawList(EntityRawPtrArray& entities, Camera* camera);

   public:
    SceneRenderPassParams m_params;

    ShadowPassPtr m_shadowPass               = nullptr;
    ForwardRenderPassPtr m_forwardRenderPass = nullptr;
    GBufferPass m_gBufferPass;
    DeferredRenderPass m_deferredRenderPass;
  };

} // namespace ToolKit
