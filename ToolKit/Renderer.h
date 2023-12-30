/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Camera.h"
#include "GpuProgram.h"
#include "Primative.h"
#include "RenderState.h"
#include "Sky.h"
#include "Types.h"
#include "Viewport.h"

namespace ToolKit
{

  class TK_API Renderer
  {
   public:
    Renderer();
    ~Renderer();

    void Init();
    void SetRenderState(const RenderState* const state);

    void SetStencilOperation(StencilOperation op);

    void SetFramebuffer(FramebufferPtr fb,
                        GraphicBitFields attachmentsToClear,
                        const Vec4& clearColor         = Vec4(0.0f),
                        GraphicFramebufferTypes fbType = GraphicFramebufferTypes::Framebuffer);

    FramebufferPtr GetFrameBuffer();
    void ClearColorBuffer(const Vec4& color);
    void ClearBuffer(GraphicBitFields fields, const Vec4& value = Vec4(0.0f));
    void ColorMask(bool r, bool g, bool b, bool a);
    void CopyFrameBuffer(FramebufferPtr src, FramebufferPtr dest, GraphicBitFields fields);
    void InvalidateFramebufferDepth(FramebufferPtr fb);
    void InvalidateFramebufferStencil(FramebufferPtr fb);
    void InvalidateFramebufferDepthStencil(FramebufferPtr fb);

    void SetViewport(Viewport* viewport);
    void SetViewportSize(uint width, uint height);
    void SetViewportSize(uint x, uint y, uint width, uint height);

    void DrawFullQuad(ShaderPtr fragmentShader);
    void DrawFullQuad(MaterialPtr mat);
    void DrawCube(CameraPtr cam, MaterialPtr mat, const Mat4& transform = Mat4(1.0f));

    void SetTexture(ubyte slotIndx, uint textureId);

    CubeMapPtr GenerateCubemapFrom2DTexture(TexturePtr texture, uint width, uint height, float exposure = 1.0f);

    CubeMapPtr GenerateSpecularEnvMap(CubeMapPtr cubemap, uint width, uint height, int mipMaps);

    CubeMapPtr GenerateDiffuseEnvMap(CubeMapPtr cubemap, uint width, uint height);

    void CopyTexture(TexturePtr source, TexturePtr dest);

    /**
     * Sets the underlying graphics api state directly which causes by passing
     * material system. Don't use it unless its necessary for special cases.
     * @param enable sets the blending on / off for the graphics api.
     */
    void EnableBlending(bool enable);

    void EnableDepthWrite(bool enable);

    void EnableDepthTest(bool enable);

    void SetDepthTestFunc(CompareFunctions func);

    // Giving nullptr as argument means no shadows
    void SetShadowAtlas(TexturePtr shadowAtlas);

    // TODO: Should be private or within a Pass.
    /////////////////////
    // Left public for thumbnail rendering. TODO: there must be techniques
    // handling thumbnail render.
    void Render(const struct RenderJob& job, CameraPtr cam, const LightPtrArray& lights = {});

    void Render(const RenderJobArray& jobArray, CameraPtr cam, const LightPtrArray& lights = {});

    void Apply7x1GaussianBlur(const TexturePtr source, RenderTargetPtr dest, const Vec3& axis, const float amount);

    void ApplyAverageBlur(const TexturePtr source, RenderTargetPtr dest, const Vec3& axis, const float amount);

    void GenerateBRDFLutTexture();

    /**
     * Sets the camera to be used for rendering. Also calculates camera related parameters, such as view, transform,
     * viewTransform etc...
     * if setLense is true sets the lens to fit aspect ratio to frame buffer.
     */
    void SetCamera(CameraPtr camera, bool setLens);
    /////////////////////

    int GetMaxArrayTextureLayers();

    void ResetTextureSlots();

   private:
    void BindProgram(GpuProgramPtr program);
    void FeedUniforms(GpuProgramPtr program);
    void FeedLightUniforms(GpuProgramPtr program);

   public:
    uint m_frameCount = 0;
    UVec2 m_windowSize; //!< Application window size.
    Vec4 m_clearColor         = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    MaterialPtr m_overrideMat = nullptr;
    CameraPtr m_uiCamera      = nullptr;
    SkyBasePtr m_sky          = nullptr;
    GpuProgramManager m_gpuProgramManager;

    bool m_renderOnlyLighting = false;

    struct RHIConstants
    {
      static constexpr ubyte TextureSlotCount      = 32;
      static constexpr ubyte MaxLightsPerObject    = 16;
      static constexpr uint ShadowAtlasSlot        = 8;
      static constexpr uint ShadowAtlasTextureSize = 2048;
      static constexpr uint SpecularIBLLods        = 5;
      static constexpr uint BrdfLutTextureSize     = 512;
      static constexpr float ShadowBiasMultiplier  = 0.0001f;
    };

   private:
    uint m_currentProgram = 0;

    // Camera data.
    CameraPtr m_cam       = nullptr;
    Mat4 m_project;
    Mat4 m_view;
    Mat4 m_projectView;
    Mat4 m_projectViewNoTranslate;
    Vec3 m_camPos;
    Vec3 m_camDirection;

    Mat4 m_model;
    Mat4 m_iblRotation;
    LightPtrArray m_lights;
    MaterialPtr m_mat            = nullptr;
    MaterialPtr m_aoMat          = nullptr;
    FramebufferPtr m_framebuffer = nullptr;
    TexturePtr m_shadowAtlas     = nullptr;

    uint m_textureSlots[RHIConstants::TextureSlotCount];

    RenderState m_renderState;

    UVec2 m_viewportSize; //!< Current viewport size.

    /*
     * This framebuffer can ONLY have 1 color attachment and no other attachments.
     * This way, we can use without needing to resize or reInit.
     */
    FramebufferPtr m_oneColorAttachmentFramebuffer = nullptr;
    MaterialPtr m_gaussianBlurMaterial             = nullptr;
    MaterialPtr m_averageBlurMaterial              = nullptr;

    FramebufferPtr m_copyFb                        = nullptr;
    MaterialPtr m_copyMaterial                     = nullptr;

    int m_maxArrayTextureLayers                    = -1;

    // Dummy objects for draw commands.
    CubePtr m_dummyDrawCube                        = nullptr;
  };

} // namespace ToolKit
