/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

    void Init();
    void SetRenderState(const RenderState* const state);

    void SetStencilOperation(StencilOperation op);
    void SetFramebuffer(FramebufferPtr fb, bool clear, const Vec4& color);
    void SetFramebuffer(FramebufferPtr fb, bool clear = true);
    void SwapFramebuffer(FramebufferPtr& fb, bool clear, const Vec4& color);
    void SwapFramebuffer(FramebufferPtr& fb, bool clear = true);

    FramebufferPtr GetFrameBuffer();
    void ClearFrameBuffer(FramebufferPtr fb, const Vec4& value);
    void ClearColorBuffer(const Vec4& color);
    void ClearBuffer(GraphicBitFields fields, const Vec4& value);
    void ColorMask(bool r, bool g, bool b, bool a);
    void CopyFrameBuffer(FramebufferPtr src, FramebufferPtr dest, GraphicBitFields fields);

    void SetViewport(Viewport* viewport);
    void SetViewportSize(uint width, uint height);
    void SetViewportSize(uint x, uint y, uint width, uint height);

    void DrawFullQuad(ShaderPtr fragmentShader);
    void DrawFullQuad(MaterialPtr mat);
    void DrawCube(Camera* cam, MaterialPtr mat, const Mat4& transform = Mat4(1.0f));

    void SetTexture(ubyte slotIndx, uint textureId);

    CubeMapPtr GenerateCubemapFrom2DTexture(TexturePtr texture, uint width, uint height, float exposure = 1.0f);

    CubeMapPtr GenerateEnvPrefilteredMap(CubeMapPtr cubemap, uint width, uint height, int mipMaps);

    CubeMapPtr GenerateEnvIrradianceMap(CubeMapPtr cubemap, uint width, uint height);

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
    void Render(const struct RenderJob& job, Camera* cam, const LightRawPtrArray& lights = {});

    void Render(const RenderJobArray& jobArray, Camera* cam, const LightRawPtrArray& lights = {});

    void Apply7x1GaussianBlur(const TexturePtr source, RenderTargetPtr dest, const Vec3& axis, const float amount);

    void ApplyAverageBlur(const TexturePtr source, RenderTargetPtr dest, const Vec3& axis, const float amount);

    /**
     * Just before the render, set the lens to fit aspect ratio to frame buffer.
     */
    void SetCameraLens(Camera* cam);
    /////////////////////

    int GetMaxArrayTextureLayers();

    void ResetTextureSlots();

   private:
    void SetProjectViewModel(const Mat4& model, Camera* cam);
    void BindProgram(ProgramPtr program);
    void LinkProgram(uint program, uint vertexP, uint fragmentP);
    ProgramPtr CreateProgram(ShaderPtr vertex, ShaderPtr fragment);
    void FeedUniforms(ProgramPtr program);
    void FeedLightUniforms(ProgramPtr program);

   public:
    uint m_frameCount = 0;
    UVec2 m_windowSize; //!< Application window size.
    Vec4 m_clearColor         = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
    MaterialPtr m_overrideMat = nullptr;
    Camera* m_uiCamera        = nullptr;
    SkyBase* m_sky            = nullptr;

    bool m_renderOnlyLighting = false;

    typedef struct RHIConstants
    {
      static constexpr ubyte textureSlotCount       = 32;
      static constexpr size_t maxLightsPerObject    = 16;
      static constexpr int shadowAtlasSlot          = 8;
      static constexpr int g_shadowAtlasTextureSize = 4096;
      static constexpr int specularIBLLods          = 5;
    } m_rhiSettings;

    static constexpr float g_shadowBiasMultiplier = 0.0001f;

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

    UVec2 m_viewportSize; //!< Current viewport size.

    /**
     * Temporary frame buffer to use in various operation. Don't rely on its
     * sate or use it to cache state.
     * Only use ColorAttachment 0 and do not init with depth buffer.
     */
    FramebufferPtr m_utilFramebuffer   = nullptr;
    MaterialPtr m_gaussianBlurMaterial = nullptr;
    MaterialPtr m_averageBlurMaterial  = nullptr;

    FramebufferPtr m_copyFb            = nullptr;
    MaterialPtr m_copyMaterial         = nullptr;

    int m_maxArrayTextureLayers        = -1;

    // Dummy objects for draw commands.
    CubePtr m_dummyDrawCube            = nullptr;
  };

} // namespace ToolKit
