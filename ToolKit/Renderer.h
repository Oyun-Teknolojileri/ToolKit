#pragma once

#include "Framebuffer.h"
#include "Light.h"
#include "RenderState.h"
#include "Sky.h"
#include "SpriteSheet.h"
#include "Types.h"
#include "Viewport.h"

#include <memory>
#include <unordered_map>

namespace ToolKit
{

  class TK_API Renderer
  {
   public:
    Renderer();
    ~Renderer();

    void RenderScene(const ScenePtr scene,
                     Viewport* viewport,
                     const LightRawPtrArray& editorLights);

    /**
     * Renders given UILayer to given Viewport.
     * @param layer UILayer that will be rendered.
     * @param viewport that UILayer will be rendered with.
     */
    void RenderUI(Viewport* viewport, UILayer* layer);

    void Render(Entity* ntt,
                Camera* cam,
                const LightRawPtrArray& editorLights = LightRawPtrArray());

    void SetRenderState(const RenderState* const state, ProgramPtr program);

    void SetFramebuffer(Framebuffer* fb, bool clear, const Vec4& color);
    void SetFramebuffer(Framebuffer* fb, bool clear = true);
    void SwapFramebuffer(Framebuffer** fb, bool clear, const Vec4& color);
    void SwapFramebuffer(Framebuffer** fb, bool clear = true);

    void SetViewport(Viewport* viewport);
    void SetViewportSize(uint width, uint height);

    void DrawFullQuad(ShaderPtr fragmentShader);
    void DrawFullQuad(MaterialPtr mat);
    void DrawCube(Camera* cam,
                  MaterialPtr mat,
                  const Mat4& transform = Mat4(1.0f));
    void SetTexture(ubyte slotIndx, uint textureId);
    void SetShadowMapTexture(EntityType type,
                             uint textureId,
                             ProgramPtr program);
    void ResetShadowMapBindings(ProgramPtr program);

    Texture* GenerateCubemapFrom2DTexture(TexturePtr texture,
                                          uint width,
                                          uint height,
                                          float exposure = 1.0f);

    Texture* GenerateIrradianceCubemap(CubeMapPtr cubemap,
                                       uint width,
                                       uint height);
    LightRawPtrArray GetBestLights(Entity* entity,
                                   const LightRawPtrArray& lights);

   private:
    void RenderEntities(
        EntityRawPtrArray& entities,
        Camera* cam,
        Viewport* viewport,
        const LightRawPtrArray& editorLights = LightRawPtrArray());

    /**
     * Removes the entites that are outside of the camera.
     * @param entities All entites.
     * @param camera Camera that is being used for generating frustum.
     */
    void FrustumCull(EntityRawPtrArray& entities, Camera* camera);

    /**
     * Extracts blended entites from given entity array.
     * @param entities Entity array that the transparents will extracted from.
     * @param blendedEntities Entity array that are going to be filled
     * with transparents.
     */
    void GetTransparentEntites(EntityRawPtrArray& entities,
                               EntityRawPtrArray& blendedEntities);

    /**
     * Renders the entities immediately. No sorting applied.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param zoom Zoom amount of camera.
     * @param editorLights All lights.
     */
    void RenderOpaque(
        EntityRawPtrArray entities,
        Camera* cam,
        float zoom,
        const LightRawPtrArray& editorLights = LightRawPtrArray());

    /**
     * Sorts and renders entities. For double-sided blended entities first
     * render back, than renders front.
     * @param entities All entities to render.
     * @param cam Camera for rendering.
     * @param zoom Zoom amount of camera.
     * @param editorLights All lights.
     */
    void RenderTransparent(
        EntityRawPtrArray entities,
        Camera* cam,
        float zoom,
        const LightRawPtrArray& editorLights = LightRawPtrArray());

    void RenderSky(SkyBase* sky, Camera* cam);

    void Render2d(Surface* object, glm::ivec2 screenDimensions);
    void Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions);

    void GetEnvironmentLightEntities(EntityRawPtrArray entities);
    void FindEnvironmentLight(Entity* entity, Camera* camera);

    void ShadowPass(const LightRawPtrArray& lights,
                    const EntityRawPtrArray& entities);
    void UpdateShadowMaps(const LightRawPtrArray& lights,
                          const EntityRawPtrArray& entities);
    void FilterShadowMaps(const LightRawPtrArray& lights);
    void Apply7x1GaussianBlur(const TexturePtr source,
                              RenderTargetPtr dest,
                              const Vec3& axis,
                              const float amount);
    void ApplyAverageBlur(const TexturePtr source,
                          RenderTargetPtr dest,
                          const Vec3& axis,
                          const float amount);

    void SetProjectViewModel(Entity* ntt, Camera* cam);
    void BindProgram(ProgramPtr program);
    void LinkProgram(uint program, uint vertexP, uint fragmentP);
    ProgramPtr CreateProgram(ShaderPtr vertex, ShaderPtr fragment);
    void FeedUniforms(ProgramPtr program);
    void FeedLightUniforms(ProgramPtr program);
    void SetVertexLayout(VertexLayout layout);

    void GenerateSSAOTexture(const EntityRawPtrArray& entities,
                             Viewport* viewport);
    void GenerateKernelAndNoiseForSSAOSamples(Vec3Array& ssaoKernel,
                                              Vec2Array& ssaoNoise);

   public:
    uint m_totalFrameCount = 0;
    uint m_frameCount      = 0;
    UVec2 m_windowSize;   //!< Application window size.
    UVec2 m_viewportSize; //!< Current viewport size.
    Vec4 m_clearColor             = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    MaterialPtr m_overrideMat     = nullptr;
    bool m_overrideDiffuseTexture = false;

    // Grid parameters
    struct GridParams
    {
      float sizeEachCell       = 0.1f;
      float maxLinePixelCount  = 2.0f;
      Vec3 axisColorHorizontal = X_AXIS;
      Vec3 axisColorVertical   = Z_AXIS;
      bool is2DViewport        = false;
    };
    GridParams m_gridParams;
    Camera* m_uiCamera = nullptr;

   private:
    uint m_currentProgram = 0;
    Mat4 m_project;
    Mat4 m_view;
    Mat4 m_model;
    Mat4 m_iblRotation;
    LightRawPtrArray m_lights;
    Camera* m_cam              = nullptr;
    Camera* m_shadowMapCamera  = nullptr;
    Material* m_mat            = nullptr;
    MaterialPtr m_aoMat        = nullptr;
    Framebuffer* m_framebuffer = nullptr;
    FramebufferSettings m_lastFramebufferSettings;

    typedef struct RHIConstants
    {
      static constexpr ubyte textureSlotCount = 8;
      // 4 studio lights, 8 in game lights
      static constexpr size_t maxLightsPerObject     = 12;
      static constexpr int maxDirAndSpotLightShadows = 4;
      static constexpr int maxPointLightShadows      = 4;
      static constexpr int maxShadows                = 8;
    } m_rhiSettings;
    uint m_textureSlots[RHIConstants::textureSlotCount];
    int m_bindedShadowMapCount       = 0;
    int m_dirAndSpotLightShadowCount = 0;
    int m_pointLightShadowCount      = 0;

    std::unordered_map<String, ProgramPtr> m_programs;
    RenderState m_renderState;

    EntityRawPtrArray m_environmentLightEntities;

    Framebuffer* m_utilFramebuffer     = nullptr;
    MaterialPtr m_gaussianBlurMaterial = nullptr;
    MaterialPtr m_averageBlurMaterial  = nullptr;
  };

} // namespace ToolKit
