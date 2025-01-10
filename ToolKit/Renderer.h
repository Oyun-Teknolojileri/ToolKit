/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Camera.h"
#include "GpuProgram.h"
#include "LightCache.h"
#include "LightDataBuffer.h"
#include "Primative.h"
#include "RHIConstants.h"
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
    void SetRenderState(const RenderState* const state, bool cullFlip = false);

    void SetStencilOperation(StencilOperation op);

    void StartTimerQuery();
    void EndTimerQuery();

    /** Returns elapsed time between start - end time query in milliseconds.*/
    void GetElapsedTime(float& cpu, float& gpu);

    void ClearColorBuffer(const Vec4& color);
    void ClearBuffer(GraphicBitFields fields, const Vec4& value = Vec4(0.0f));
    void ColorMask(bool r, bool g, bool b, bool a);

    // FrameBuffer Operations
    //////////////////////////////////////////

    FramebufferPtr GetFrameBuffer();

    void SetFramebuffer(FramebufferPtr fb,
                        GraphicBitFields attachmentsToClear,
                        const Vec4& clearColor         = Vec4(0.0f),
                        GraphicFramebufferTypes fbType = GraphicFramebufferTypes::Framebuffer);

    void InvalidateFramebufferDepth(FramebufferPtr frameBuffer);
    void InvalidateFramebufferStencil(FramebufferPtr frameBuffer);
    void InvalidateFramebufferDepthStencil(FramebufferPtr frameBuffer);

    /**
     * Sets the src and dest frame buffers and copies the given fields.
     * After the operation sets the previous frame buffer back.
     */
    void CopyFrameBuffer(FramebufferPtr src, FramebufferPtr dest, GraphicBitFields fields);

    /**
     * Copies src to dst texture using a copy frame buffer.
     * After the operation sets the previous frame buffer back.
     */
    void CopyTexture(TexturePtr src, TexturePtr dst);

    //////////////////////////////////////////

    void SetViewport(Viewport* viewport);
    void SetViewportSize(uint width, uint height);
    void SetViewportSize(uint x, uint y, uint width, uint height);

    void DrawFullQuad(ShaderPtr fragmentShader);
    void DrawFullQuad(MaterialPtr mat);
    void DrawCube(CameraPtr cam, MaterialPtr mat, const Mat4& transform = Mat4(1.0f));

    void SetTexture(ubyte slotIndx, uint textureId);

    CubeMapPtr GenerateCubemapFrom2DTexture(TexturePtr texture, uint size, float exposure = 1.0f);
    CubeMapPtr GenerateSpecularEnvMap(CubeMapPtr cubemap, uint size, int mipMaps);
    CubeMapPtr GenerateDiffuseEnvMap(CubeMapPtr cubemap, uint size);

    /**
     * Sets the blend state directly which causes by passing material system.
     * @param enableOverride when set true, disables the material system setting blend state per material.
     * @param func is the BlendFunction to use.
     */
    void OverrideBlendState(bool enableOverride, BlendFunction func);

    void EnableBlending(bool enable);
    void EnableDepthWrite(bool enable);
    void EnableDepthTest(bool enable);
    void SetDepthTestFunc(CompareFunctions func);

    // Giving nullptr as argument means no shadows
    void SetShadowAtlas(TexturePtr shadowAtlas);

    void Render(const struct RenderJob& job);
    void Render(const RenderJobArray& jobs);

    void RenderWithProgramFromMaterial(const RenderJobArray& jobs);
    void RenderWithProgramFromMaterial(const RenderJob& job);

    /** Apply one tap of gauss blur via setting a temporary frame buffer. Does not reset frame buffer back. */
    void Apply7x1GaussianBlur(const TexturePtr src, RenderTargetPtr dst, const Vec3& axis, const float amount);
    /** Apply one tap of average blur via setting a temporary frame buffer. Does not reset frame buffer back. */
    void ApplyAverageBlur(const TexturePtr src, RenderTargetPtr dst, const Vec3& axis, const float amount);

    /**
     * Sets the camera to be used for rendering. Also calculates camera related parameters, such as view, transform,
     * viewTransform etc...
     * if setLense is true sets the lens to fit aspect ratio to frame buffer.
     * Invalidates gpu program's related caches.
     */
    void SetCamera(CameraPtr camera, bool setLens);

    int GetMaxArrayTextureLayers();
    void BindProgramOfMaterial(Material* material);
    void BindProgram(const GpuProgramPtr& program);
    void ResetUsedTextureSlots();

    /** Initialize brdf lut textures. */
    void GenerateBRDFLutTexture();

    /** Ambient occlusion texture to be applied. If ao is not enabled, set this explicitly to null. */
    void SetAmbientOcclusionTexture(TexturePtr aoTexture);

   private:
    void FeedUniforms(const GpuProgramPtr& program, const RenderJob& job);
    void FeedLightUniforms(const GpuProgramPtr& program, const RenderJob& job);
    void FeedAnimationUniforms(const GpuProgramPtr& program, const RenderJob& job);

   public:
    uint m_frameCount = 0;
    UVec2 m_windowSize; //!< Application window size.
    Vec4 m_clearColor       = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    CameraPtr m_uiCamera    = nullptr;
    CameraPtr m_tempQuadCam = nullptr;
    SkyBasePtr m_sky        = nullptr;

    // The set contains gpuPrograms that has up to date camera uniforms.
    std::unordered_set<uint> m_gpuProgramHasCameraUpdates;

    // The set contains gpuPrograms that has up to date per frame uniforms.
    std::unordered_set<uint> m_gpuProgramHasFrameUpdates;

    bool m_renderOnlyLighting = false;
    LightCache<RHIConstants::LightCacheSize> m_lightCache;

   private:
    LightDataBuffer m_lightDataBuffer;
    uint16 m_drawCallVersion       = 1;

    GpuProgramPtr m_currentProgram = nullptr;

    // Camera data.
    CameraPtr m_cam                = nullptr;
    Mat4 m_project;
    Mat4 m_view;
    Mat4 m_projectView;
    Mat4 m_projectViewNoTranslate;
    Vec3 m_camPos;
    Vec3 m_camDirection;
    float m_camFar = 0.1f;

    Mat4 m_model;
    Mat4 m_iblRotation;
    Material* m_mat              = nullptr;
    FramebufferPtr m_framebuffer = nullptr;
    TexturePtr m_shadowAtlas     = nullptr;
    RenderTargetPtr m_brdfLut    = nullptr;
    TexturePtr m_aoTexture       = nullptr;

    int m_textureSlots[RHIConstants::TextureSlotCount];

    RenderState m_renderState;

    UVec2 m_viewportSize; //!< Current viewport size.

    /*
     * This framebuffer can ONLY have 1 color attachment and no other attachments.
     * This way, we can use without needing to resize or reInit.
     */
    FramebufferPtr m_oneColorAttachmentFramebuffer = nullptr;
    MaterialPtr m_gaussianBlurMaterial             = nullptr;
    MaterialPtr m_averageBlurMaterial              = nullptr;
    QuadPtr m_tempQuad                             = nullptr;
    MaterialPtr m_tempQuadMaterial                 = nullptr;

    FramebufferPtr m_copyFb                        = nullptr;
    MaterialPtr m_copyMaterial                     = nullptr;

    int m_maxArrayTextureLayers                    = -1;

    // Dummy objects for draw commands.
    CubePtr m_dummyDrawCube                        = nullptr;

    GpuProgramManager* m_gpuProgramManager         = nullptr;

    uint m_gpuTimerQuery                           = 0;
    float m_cpuTime                                = 0.0f;
    bool m_blendStateOverrideEnable                = false;
  };
} // namespace ToolKit
