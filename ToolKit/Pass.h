#pragma once

#include "BinPack2D.h"
#include "DataTexture.h"
#include "Framebuffer.h"
#include "GeometryTypes.h"
#include "Primative.h"
#include "Renderer.h"

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

    Renderer* GetRenderer();
    void SetRenderer(Renderer* renderer);

   protected:
    MaterialPtr m_prevOverrideMaterial = nullptr;
    FramebufferPtr m_prevFrameBuffer   = nullptr;

   private:
    Renderer* m_renderer = nullptr;
  };

  typedef std::shared_ptr<Pass> PassPtr;
  typedef std::vector<PassPtr> PassPtrArray;

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
    ~ForwardRenderPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   protected:
    void CullLightList(const Entity* entity, LightRawPtrArray& lights);

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

    /**
     * Sets layer and coordinates of the shadow maps in shadow atlas.
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
    const Vec4 m_shadowClearColor      = Vec4(1.0f);

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
    LightRawPtrArray lights;
    FramebufferPtr FrameBuffer = nullptr;
    BlendFunction BlendFunc    = BlendFunction::NONE;
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

  struct CubeMapPassParams
  {
    FramebufferPtr FrameBuffer = nullptr;
    Camera* Cam                = nullptr;
    MaterialPtr Material       = nullptr;
    Mat4 Transform;
  };

  class TK_API CubeMapPass : public Pass
  {
   public:
    CubeMapPass();
    explicit CubeMapPass(const CubeMapPassParams& params);
    ~CubeMapPass();

    void Render() override;
    void PreRender() override;
    void PostRender() override;

   public:
    CubeMapPassParams m_params;

   private:
    CubePtr m_cube = nullptr;
  };

  typedef std::shared_ptr<CubeMapPass> CubeMapPassPtr;

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
    ~StencilRenderPass();

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
    ~OutlinePass();

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

  struct GBufferPassParams
  {
    EntityRawPtrArray entities;
    Camera* camera;
  };

  class TK_API GBufferPass : public Pass
  {
   public:
    GBufferPass();
    explicit GBufferPass(const GBufferPassParams& params);
    ~GBufferPass();

    void PreRender() override;
    void PostRender() override;
    void Render() override;
    void InitGBuffers(int width, int height);
    void UnInitGBuffers();

   public:
    FramebufferPtr m_framebuffer           = nullptr;
    RenderTargetPtr m_gPosRt               = nullptr;
    RenderTargetPtr m_gNormalRt            = nullptr;
    RenderTargetPtr m_gColorRt             = nullptr;
    RenderTargetPtr m_gEmissiveRt          = nullptr;
    RenderTargetPtr m_gLinearDepthRt       = nullptr;
    RenderTargetPtr m_gMetallicRoughnessRt = nullptr;

    int m_width                            = 1024;
    int m_height                           = 1024;

    GBufferPassParams m_params;

   private:
    bool m_initialized            = false;
    bool m_attachmentsSet         = false;
    MaterialPtr m_gBufferMaterial = nullptr;
  };

  typedef std::shared_ptr<GBufferPass> GBufferPassPtr;

  struct DeferredRenderPassParams
  {
    LightRawPtrArray lights;
    FramebufferPtr MainFramebuffer    = nullptr;
    FramebufferPtr GBufferFramebuffer = nullptr;
    bool ClearFramebuffer             = true;
    Camera* Cam                       = nullptr;
    TexturePtr AOTexture              = nullptr;
  };

  class TK_API DeferredRenderPass : public RenderPass
  {
   public:
    DeferredRenderPass();
    DeferredRenderPass(const DeferredRenderPassParams& params);
    ~DeferredRenderPass();

    void PreRender() override;
    void PostRender() override;
    void Render() override;

   private:
    void InitLightDataTexture();

   public:
    DeferredRenderPassParams m_params;

   private:
    FullQuadPassPtr m_fullQuadPass         = nullptr;
    ShaderPtr m_deferredRenderShader       = nullptr;
    LightDataTexturePtr m_lightDataTexture = nullptr;
    const IVec2 m_lightDataTextureSize     = IVec2(1024);
  };

  typedef std::shared_ptr<DeferredRenderPass> DeferredRenderPassPtr;

  struct SSAOPassParams
  {
    TexturePtr GPositionBuffer    = nullptr;
    TexturePtr GNormalBuffer      = nullptr;
    TexturePtr GLinearDepthBuffer = nullptr;
    Camera* Cam                   = nullptr;
  };

  class TK_API SSAOPass : public Pass
  {
   public:
    SSAOPass();
    explicit SSAOPass(const SSAOPassParams& params);
    ~SSAOPass();

    void Render();
    void PreRender();
    void PostRender();

   private:
    void GenerateSSAONoise();

   public:
    SSAOPassParams m_params;
    RenderTargetPtr m_ssaoTexture = nullptr;

   private:
    Vec3Array m_ssaoKernel;
    Vec2Array m_ssaoNoise;

    FramebufferPtr m_ssaoFramebuffer   = nullptr;
    SSAONoiseTexturePtr m_noiseTexture = nullptr;
    RenderTargetPtr m_tempBlurRt       = nullptr;

    FullQuadPassPtr m_quadPass         = nullptr;
    ShaderPtr m_ssaoShader             = nullptr;
  };

  typedef std::shared_ptr<SSAOPass> SSAOPassPtr;

} // namespace ToolKit
