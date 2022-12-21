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

  /**
   * Simple binary stencil test operations.
   */
  enum class StencilOperation
  {
    /**
     * Stencil write and operations are disabled.
     */
    None,
    /**
     * All pixels are drawn and stencil value of the corresponding pixel set
     * to 1.
     */
    AllowAllPixels,
    /**
     * Pixels whose stencil value is 1 are drawn.
     */
    AllowPixelsPassingStencil,
    /**
     * Pixels whose stencil value is 0 are drawn.
     */
    AllowPixelsFailingStencil
  };

  class TK_API Renderer
  {
   public:
    Renderer();
    ~Renderer();

    /**
     * Renders given UILayer to given Viewport.
     * @param layer UILayer that will be rendered.
     * @param viewport that UILayer will be rendered with.
     */
    void RenderUI(Viewport* viewport, UILayer* layer);

    void SetRenderState(const RenderState* const state, ProgramPtr program);

    void SetStencilOperation(StencilOperation op);
    void SetFramebuffer(FramebufferPtr fb, bool clear, const Vec4& color);
    void SetFramebuffer(FramebufferPtr fb, bool clear = true);
    void SwapFramebuffer(FramebufferPtr& fb, bool clear, const Vec4& color);
    void SwapFramebuffer(FramebufferPtr& fb, bool clear = true);

    FramebufferPtr GetFrameBuffer();
    void ClearFrameBuffer(FramebufferPtr fb, const Vec4& color);
    void ClearColorBuffer(const Vec4& value);
    void ClearBuffer(GraphicBitFields fields);
    void ColorMask(bool r, bool g, bool b, bool a);
    void CopyFrameBuffer(FramebufferPtr src,
                         FramebufferPtr dest,
                         GraphicBitFields fields);

    void SetViewport(Viewport* viewport);
    void SetViewportSize(uint width, uint height);
    void SetViewportSize(uint x, uint y, uint width, uint height);

    void DrawFullQuad(ShaderPtr fragmentShader);
    void DrawFullQuad(MaterialPtr mat);
    void DrawCube(Camera* cam,
                  MaterialPtr mat,
                  const Mat4& transform = Mat4(1.0f));

    void SetTexture(ubyte slotIndx, uint textureId);

    CubeMapPtr GenerateCubemapFrom2DTexture(TexturePtr texture,
                                            uint width,
                                            uint height,
                                            float exposure = 1.0f);

    CubeMapPtr GenerateIrradianceCubemap(CubeMapPtr cubemap,
                                         uint width,
                                         uint height);

    LightRawPtrArray GetBestLights(Entity* entity,
                                   const LightRawPtrArray& lights);

    void CopyTexture(TexturePtr source, TexturePtr dest);

    /**
     * Sets the underlying graphics api state directly which causes by passing
     * material system. Don't use it unless its necessary for special cases.
     * @param enable sets the blending on / off for the graphics api.
     */
    void EnableBlending(bool enable);

    // If there is a material component, returns its material else
    // returns mesh's material. If there is not a MaterialComponent, it will
    // return the mesh's first material. In case of multisubmesh, there may be
    // multiple materials. But they are ignored.
    MaterialPtr GetRenderMaterial(Entity* entity);

    // Giving nullptr as argument means no shadows
    void SetShadowAtlas(TexturePtr shadowAtlas);

    // TODO: Should be private or within a Pass.
    /////////////////////
    // Left public for thumbnail rendering. TODO: there must be techniques
    // handling thumbnail render.
    void Render(Entity* ntt,
                Camera* cam,
                const LightRawPtrArray& editorLights = LightRawPtrArray());

    void Apply7x1GaussianBlur(const TexturePtr source,
                              RenderTargetPtr dest,
                              const Vec3& axis,
                              const float amount);

    /**
     * Just before the render, set the lens to fit aspect ratio to frame buffer.
     */
    void SetCameraLens(Camera* cam);

    /**
     * Collects all the environment volumes.
     */
    void CollectEnvironmentVolumes(const EntityRawPtrArray& entities);

    /////////////////////

    int GetMaxArrayTextureLayers();

   private:
    void Render2d(Surface* object, glm::ivec2 screenDimensions);
    void Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions);

    /**
     * Sets current entities material iblInUse parameter, if it falls within an
     * environment volume or sky. Set m_iblRotation from the found environment
     * volume to reflect environment rotation.
     * @param entity to find the environment volume.
     */
    void FindEnvironmentLight(Entity* entity);

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

    void GenerateSSAOTexture(const EntityRawPtrArray& entities,
                             Viewport* viewport);
    void GenerateKernelAndNoiseForSSAOSamples(Vec3Array& ssaoKernel,
                                              Vec2Array& ssaoNoise);

   public:
    uint m_totalFrameCount = 0;
    uint m_frameCount      = 0;
    UVec2 m_windowSize; //!< Application window size.
    Vec4 m_clearColor         = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    MaterialPtr m_overrideMat = nullptr;
    Camera* m_uiCamera        = nullptr;

    bool m_renderOnlyLighting = false;

    typedef struct RHIConstants
    {
      static constexpr ubyte textureSlotCount = 16;

      static constexpr size_t maxLightsPerObject = 16;

      static constexpr int shadowAtlasSlot          = 8;
      static constexpr int g_shadowAtlasTextureSize = 4096;
    } m_rhiSettings;

   private:
    uint m_currentProgram = 0;
    Mat4 m_project;
    Mat4 m_view;
    Mat4 m_model;
    Mat4 m_iblRotation;
    LightRawPtrArray m_lights;
    Camera* m_cam                = nullptr;
    MaterialPtr m_mat            = nullptr;
    MaterialPtr m_aoMat          = nullptr;
    FramebufferPtr m_framebuffer = nullptr;
    TexturePtr m_shadowAtlas     = nullptr;

    uint m_textureSlots[RHIConstants::textureSlotCount];

    std::unordered_map<String, ProgramPtr> m_programs;
    RenderState m_renderState;

    EntityRawPtrArray m_environmentLightEntities;

    UVec2 m_viewportSize; //!< Current viewport size.

    /**
     * Temporary frame buffer to use in various operation. Don't rely on its
     * sate or use it to cache state.
     */
    FramebufferPtr m_utilFramebuffer   = nullptr;
    MaterialPtr m_gaussianBlurMaterial = nullptr;
    MaterialPtr m_averageBlurMaterial  = nullptr;

    FramebufferPtr m_copyFb    = nullptr;
    MaterialPtr m_copyMaterial = nullptr;

    int m_maxArrayTextureLayers = -1;
  };

} // namespace ToolKit
