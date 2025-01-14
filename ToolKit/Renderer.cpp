/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Renderer.h"

#include "AABBOverrideComponent.h"
#include "Camera.h"
#include "DirectionComponent.h"
#include "Drawable.h"
#include "EngineSettings.h"
#include "EnvironmentComponent.h"
#include "GradientSky.h"
#include "Logger.h"
#include "Material.h"
#include "MathUtil.h"
#include "Mesh.h"
#include "Node.h"
#include "Pass.h"
#include "RHI.h"
#include "RenderSystem.h"
#include "Scene.h"
#include "Shader.h"
#include "Skeleton.h"
#include "Surface.h"
#include "TKAssert.h"
#include "TKOpenGL.h"
#include "TKStats.h"
#include "Texture.h"
#include "ToolKit.h"
#include "UIManager.h"
#include "Viewport.h"

namespace ToolKit
{
  Renderer::Renderer() {}

  void Renderer::Init()
  {
    m_uiCamera                      = MakeNewPtr<Camera>();
    m_oneColorAttachmentFramebuffer = MakeNewPtr<Framebuffer>("RendererOneColorFB");
    m_dummyDrawCube                 = MakeNewPtr<Cube>();

    m_gpuProgramManager             = GetGpuProgramManager();

    m_lightDataBuffer.Init();

    glGenQueries(1, &m_gpuTimerQuery);

    const char* renderer = (const char*) glGetString(GL_RENDERER);
    GetLogger()->Log(String("Graphics Card ") + renderer);

    // Default states.
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    glClearDepthf(1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  }

  Renderer::~Renderer()
  {
    m_oneColorAttachmentFramebuffer = nullptr;
    m_gaussianBlurMaterial          = nullptr;
    m_averageBlurMaterial           = nullptr;
    m_copyFb                        = nullptr;
    m_copyMaterial                  = nullptr;

    m_mat                           = nullptr;
    m_framebuffer                   = nullptr;
    m_shadowAtlas                   = nullptr;

    m_lightDataBuffer.Destroy();
  }

  int Renderer::GetMaxArrayTextureLayers()
  {
    if (m_maxArrayTextureLayers == -1)
    {
      glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_maxArrayTextureLayers);
    }
    return m_maxArrayTextureLayers;
  }

  void Renderer::SetCamera(CameraPtr camera, bool setLens)
  {
    m_gpuProgramHasCameraUpdates.clear();

    if (setLens)
    {
      float aspect = (float) m_viewportSize.x / (float) m_viewportSize.y;
      if (!camera->IsOrtographic())
      {
        camera->SetLens(camera->Fov(), aspect, camera->Near(), camera->Far());
      }
      else
      {
        float width  = m_viewportSize.x * 0.5f;
        float height = m_viewportSize.y * 0.5f;
        camera->SetLens(-width, width, -height, height, camera->Near(), camera->Far());
      }
    }

    m_cam                    = camera;
    m_project                = camera->GetProjectionMatrix();
    m_view                   = camera->GetViewMatrix();
    m_projectView            = m_project * m_view;

    Mat4 viewNoTr            = m_view;
    viewNoTr[0][3]           = 0.0f;
    viewNoTr[1][3]           = 0.0f;
    viewNoTr[2][3]           = 0.0f;
    viewNoTr[3][3]           = 1.0f;
    viewNoTr[3][0]           = 0.0f;
    viewNoTr[3][1]           = 0.0f;
    viewNoTr[3][2]           = 0.0f;

    m_projectViewNoTranslate = m_project * viewNoTr;

    m_camPos                 = m_cam->Position();
    m_camDirection           = m_cam->Direction();
    m_camFar                 = m_cam->Far();
  }

  void Renderer::Render(const RenderJob& job)
  {
    // Make ibl assignments.
    m_renderState.IBLInUse = false;
    if (job.EnvironmentVolume)
    {
      const EnvironmentComponent* envCom = job.EnvironmentVolume;
      m_renderState.iblIntensity         = envCom->GetIntensityVal();

      const HdriPtr& hdriPtr             = envCom->GetHdriVal();
      CubeMapPtr& diffuseEnvMap          = hdriPtr->m_diffuseEnvMap;
      CubeMapPtr& specularEnvMap         = hdriPtr->m_specularEnvMap;

      if (diffuseEnvMap && specularEnvMap && m_brdfLut)
      {
        m_renderState.irradianceMap          = diffuseEnvMap->m_textureId;
        m_renderState.preFilteredSpecularMap = specularEnvMap->m_textureId;
        m_renderState.brdfLut                = m_brdfLut->m_textureId;

        m_renderState.IBLInUse               = true;
        if (const EntityPtr& env = envCom->OwnerEntity())
        {
          m_iblRotation = Mat4(env->m_node->GetOrientation());
        }
      }
    }

    // Skeleton Component is used by all meshes of an entity.
    const auto& updateAndBindSkinningTextures = [&]()
    {
      if (!job.Mesh->IsSkinned())
      {
        return;
      }

      const SkeletonPtr& skel = static_cast<SkinMesh*>(job.Mesh)->m_skeleton;
      if (skel == nullptr)
      {
        return;
      }

      if (job.animData.currentAnimation != nullptr)
      {
        // animation.
        AnimationPlayer* animPlayer = GetAnimationPlayer();
        DataTexturePtr animTexture =
            animPlayer->GetAnimationDataTexture(skel->GetIdVal(), job.animData.currentAnimation->GetIdVal());

        if (animTexture != nullptr)
        {
          SetTexture(3, animTexture->m_textureId);
        }

        // animation to blend.
        if (job.animData.blendAnimation != nullptr)
        {
          animTexture = animPlayer->GetAnimationDataTexture(skel->GetIdVal(), job.animData.blendAnimation->GetIdVal());
          SetTexture(2, animTexture->m_textureId);
        }
      }
      else
      {
        SetTexture(3, skel->m_bindPoseTexture->m_textureId);
      }
    };

    updateAndBindSkinningTextures();

    m_model = job.WorldTransform;
    job.Mesh->Init();
    job.Material->Init();

    // Set render material.
    m_mat = job.Material;
    m_mat->Init();

    RenderState* renderState = m_mat->GetRenderState();
    SetRenderState(renderState, job.requireCullFlip);

    auto activateSkinning = [&](const Mesh* mesh)
    {
      GLint isSkinnedLoc = m_currentProgram->GetDefaultUniformLocation(Uniform::IS_SKINNED);
      bool isSkinned     = mesh->IsSkinned();
      if (isSkinned)
      {
        SkeletonPtr skel = static_cast<SkinMesh*>(job.Mesh)->m_skeleton;
        assert(skel != nullptr);

        GLint numBonesLoc = m_currentProgram->GetDefaultUniformLocation(Uniform::NUM_BONES);
        glUniform1ui(isSkinnedLoc, 1);

        GLuint boneCount = (GLuint) skel->m_bones.size();
        glUniform1f(numBonesLoc, (float) boneCount);
      }
      else
      {
        glUniform1ui(isSkinnedLoc, 0);
      }
    };

    const Mesh* mesh = job.Mesh;
    activateSkinning(mesh);

    FeedAnimationUniforms(m_currentProgram, job);
    FeedLightUniforms(m_currentProgram, job);
    FeedUniforms(m_currentProgram, job);

    RHI::BindVertexArray(mesh->m_vaoId);

    if (mesh->m_indexCount != 0)
    {
      glDrawElements((GLenum) renderState->drawType, mesh->m_indexCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
      glDrawArrays((GLenum) renderState->drawType, 0, mesh->m_vertexCount);
    }

    Stats::AddDrawCall();
  }

  void Renderer::RenderWithProgramFromMaterial(const RenderJobArray& jobs)
  {
    for (int i = 0; i < jobs.size(); ++i)
    {
      RenderWithProgramFromMaterial(jobs[i]);
    }
  }

  void Renderer::RenderWithProgramFromMaterial(const RenderJob& job)
  {
    job.Material->Init();
    GpuProgramPtr program =
        m_gpuProgramManager->CreateProgram(job.Material->m_vertexShader, job.Material->m_fragmentShader);
    BindProgram(program);
    Render(job);
  }

  void Renderer::Render(const RenderJobArray& jobs)
  {
    for (const RenderJob& job : jobs)
    {
      Render(job);
    }
  }

  void Renderer::SetRenderState(const RenderState* const state, bool cullFlip)
  {
    CullingType targetMode = state->cullMode;
    if (cullFlip)
    {
      switch (state->cullMode)
      {
      case CullingType::Front:
        targetMode = CullingType::Back;
        break;
      case CullingType::Back:
        targetMode = CullingType::Front;
        break;
      }
    }

    if (m_renderState.cullMode != targetMode)
    {
      if (targetMode == CullingType::TwoSided)
      {
        glDisable(GL_CULL_FACE);
      }

      if (targetMode == CullingType::Front)
      {
        if (m_renderState.cullMode == CullingType::TwoSided)
        {
          glEnable(GL_CULL_FACE);
        }

        glCullFace(GL_FRONT);
      }

      if (targetMode == CullingType::Back)
      {
        if (m_renderState.cullMode == CullingType::TwoSided)
        {
          glEnable(GL_CULL_FACE);
        }

        glCullFace(GL_BACK);
      }

      m_renderState.cullMode = targetMode;
    }

    if (m_renderState.blendFunction != state->blendFunction)
    {
      // Only update blend state, if blend state is not overridden.
      if (!m_blendStateOverrideEnable)
      {
        switch (state->blendFunction)
        {
        case BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA:
        {
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }
        break;
        case BlendFunction::ONE_TO_ONE:
        {
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ONE);
          glBlendEquation(GL_FUNC_ADD);
        }
        break;
        default:
        {
          glDisable(GL_BLEND);
        }
        break;
        }

        m_renderState.blendFunction = state->blendFunction;
      }
    }

    m_renderState.alphaMaskTreshold = state->alphaMaskTreshold;

    if (m_renderState.lineWidth != state->lineWidth)
    {
      m_renderState.lineWidth = state->lineWidth;
      glLineWidth(m_renderState.lineWidth);
    }

    if (m_mat)
    {
      if (m_mat->m_diffuseTexture)
      {
        SetTexture(0, m_mat->m_diffuseTexture->m_textureId);
      }

      if (m_mat->m_cubeMap)
      {
        SetTexture(6, m_mat->m_cubeMap->m_textureId);
      }
    }
  }

  void Renderer::SetStencilOperation(StencilOperation op)
  {
    switch (op)
    {
    case StencilOperation::None:
      glDisable(GL_STENCIL_TEST);
      glStencilMask(0x00);
      break;
    case StencilOperation::AllowAllPixels:
      glEnable(GL_STENCIL_TEST);
      glStencilMask(0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
      break;
    case StencilOperation::AllowPixelsPassingStencil:
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_EQUAL, 0xFF, 0xFF);
      glStencilMask(0x00);
      break;
    case StencilOperation::AllowPixelsFailingStencil:
      glEnable(GL_STENCIL_TEST);
      glStencilFunc(GL_NOTEQUAL, 0xFF, 0xFF);
      glStencilMask(0x00);
      break;
    }
  }

  void Renderer::SetFramebuffer(FramebufferPtr fb,
                                GraphicBitFields attachmentsToClear,
                                const Vec4& clearColor,
                                GraphicFramebufferTypes fbType)
  {
    if (fb != nullptr)
    {
      RHI::SetFramebuffer((GLenum) fbType, fb->GetFboId());

      const FramebufferSettings& fbSet = fb->GetSettings();
      SetViewportSize(fbSet.width, fbSet.height);
    }
    else
    {
      // Backbuffer
      RHI::SetFramebuffer((GLenum) fbType, 0);

      SetViewportSize(m_windowSize.x, m_windowSize.y);
    }

    if (attachmentsToClear != GraphicBitFields::None)
    {
      ClearBuffer(attachmentsToClear, clearColor);
    }

    m_framebuffer = fb;
  }

  void Renderer::StartTimerQuery()
  {
    m_cpuTime = GetElapsedMilliSeconds();
    if (GetEngineSettings().Graphics.enableGpuTimer)
    {
#ifdef TK_WIN
      glBeginQuery(GL_TIME_ELAPSED_EXT, m_gpuTimerQuery);
#endif
    }
  }

  void Renderer::EndTimerQuery()
  {
    float cpuTime = GetElapsedMilliSeconds();
    m_cpuTime     = cpuTime - m_cpuTime;
    if (GetEngineSettings().Graphics.enableGpuTimer)
    {
#ifdef TK_WIN
      glEndQuery(GL_TIME_ELAPSED_EXT);
#endif
    }
  }

  void Renderer::GetElapsedTime(float& cpu, float& gpu)
  {
    cpu = m_cpuTime;
    gpu = 1.0f;
#ifdef TK_WIN
    if (GetEngineSettings().Graphics.enableGpuTimer)
    {
      GLuint elapsedTime;
      glGetQueryObjectuiv(m_gpuTimerQuery, GL_QUERY_RESULT, &elapsedTime);

      gpu = glm::max(1.0f, (float) (elapsedTime) / 1000000.0f);
    }
#endif
  }

  FramebufferPtr Renderer::GetFrameBuffer() { return m_framebuffer; }

  void Renderer::ClearColorBuffer(const Vec4& color)
  {
    glClearColor(color.x, color.y, color.z, color.w);
    glClear((GLbitfield) GraphicBitFields::ColorBits);
  }

  void Renderer::ClearBuffer(GraphicBitFields fields, const Vec4& value)
  {
    glClearColor(value.x, value.y, value.z, value.w);
    glClear((GLbitfield) fields);
  }

  void Renderer::ColorMask(bool r, bool g, bool b, bool a) { glColorMask(r, g, b, a); }

  void Renderer::CopyFrameBuffer(FramebufferPtr src, FramebufferPtr dest, GraphicBitFields fields)
  {
    GLuint srcId = 0;
    uint width   = m_windowSize.x;
    uint height  = m_windowSize.y;

    if (src)
    {
      const FramebufferSettings& fbs = src->GetSettings();
      width                          = fbs.width;
      height                         = fbs.height;
      srcId                          = src->GetFboId();
    }

    dest->ReconstructIfNeeded(width, height);

    FramebufferPtr lastFb = m_framebuffer;

    RHI::SetFramebuffer(GL_READ_FRAMEBUFFER, srcId);
    RHI::SetFramebuffer(GL_DRAW_FRAMEBUFFER, dest->GetFboId());

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, (GLbitfield) fields, GL_NEAREST);

    SetFramebuffer(lastFb, GraphicBitFields::None);
  }

  // By invalidating the frame buffers attachment, bandwith and performance saving is aimed,
  // Nvidia driver issue makes the invalidate perform much worse. Clear will work the same in terms of bandwith saving
  // with no performance penalty.
#define PREFER_CLEAR_OVER_INVALIDATE 1

  void Renderer::InvalidateFramebufferDepth(FramebufferPtr frameBuffer)
  {
#if PREFER_CLEAR_OVER_INVALIDATE
    SetFramebuffer(frameBuffer, GraphicBitFields::DepthBits);
#else
    constexpr GLenum invalidAttachments[1] = {GL_DEPTH_ATTACHMENT};

    SetFramebuffer(frameBuffer, GraphicBitFields::None);
    RHI::InvalidateFramebuffer(GL_FRAMEBUFFER, 1, invalidAttachments);
#endif
  }

  void Renderer::InvalidateFramebufferStencil(FramebufferPtr frameBuffer)
  {
#if PREFER_CLEAR_OVER_INVALIDATE
    SetFramebuffer(frameBuffer, GraphicBitFields::StencilBits);
#else
    constexpr GLenum invalidAttachments[1] = {GL_STENCIL_ATTACHMENT};

    SetFramebuffer(frameBuffer, GraphicBitFields::None);
    RHI::InvalidateFramebuffer(GL_FRAMEBUFFER, 1, invalidAttachments);
#endif
  }

  void Renderer::InvalidateFramebufferDepthStencil(FramebufferPtr frameBuffer)
  {
#if PREFER_CLEAR_OVER_INVALIDATE
    SetFramebuffer(frameBuffer, GraphicBitFields::DepthStencilBits);
#else
    constexpr GLenum invalidAttachments[2] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};

    SetFramebuffer(frameBuffer, GraphicBitFields::None);
    RHI::InvalidateFramebuffer(GL_FRAMEBUFFER, 2, invalidAttachments);
#endif
  }

  void Renderer::SetViewport(Viewport* viewport) { SetFramebuffer(viewport->m_framebuffer, GraphicBitFields::AllBits); }

  void Renderer::SetViewportSize(uint width, uint height)
  {
    if (width == m_viewportSize.x && height == m_viewportSize.y)
    {
      return;
    }

    m_viewportSize.x = width;
    m_viewportSize.y = height;
    glViewport(0, 0, width, height);
  }

  void Renderer::SetViewportSize(uint x, uint y, uint width, uint height)
  {
    m_viewportSize.x = width;
    m_viewportSize.y = height;
    glViewport(x, y, width, height);
  }

  void Renderer::DrawFullQuad(ShaderPtr fragmentShader)
  {
    ShaderPtr fullQuadVert = GetShaderManager()->Create<Shader>(ShaderPath("fullQuadVert.shader", true));
    if (m_tempQuadMaterial == nullptr)
    {
      m_tempQuadMaterial = MakeNewPtr<Material>();
    }
    m_tempQuadMaterial->UnInit();

    m_tempQuadMaterial->m_vertexShader   = fullQuadVert;
    m_tempQuadMaterial->m_fragmentShader = fragmentShader;
    m_tempQuadMaterial->Init();

    DrawFullQuad(m_tempQuadMaterial);
  }

  void Renderer::DrawFullQuad(MaterialPtr mat)
  {
    if (m_tempQuad == nullptr)
    {
      m_tempQuad = MakeNewPtr<Quad>();
    }
    m_tempQuad->GetMeshComponent()->GetMeshVal()->m_material = mat;

    if (m_tempQuadCam == nullptr)
    {
      m_tempQuadCam = MakeNewPtr<Camera>();
    }
    SetCamera(m_tempQuadCam, true);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs(jobs, m_tempQuad);

    CompareFunctions lastDepthFunc = m_renderState.depthFunction;
    SetDepthTestFunc(CompareFunctions::FuncAlways);

    RenderWithProgramFromMaterial(jobs);

    SetDepthTestFunc(lastDepthFunc);
  }

  void Renderer::DrawCube(CameraPtr cam, MaterialPtr mat, const Mat4& transform)
  {
    m_dummyDrawCube->m_node->SetTransform(transform);
    m_dummyDrawCube->GetMaterialComponent()->SetFirstMaterial(mat);
    SetCamera(cam, true);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs(jobs, m_dummyDrawCube);

    CompareFunctions lastCompareFunc = m_renderState.depthFunction;
    SetDepthTestFunc(CompareFunctions::FuncAlways);

    RenderWithProgramFromMaterial(jobs);

    SetDepthTestFunc(lastCompareFunc);
  }

  void Renderer::CopyTexture(TexturePtr src, TexturePtr dst)
  {
    assert(src->m_initiated && dst->m_initiated && "Texture is not initialized.");
    assert(src->m_width == dst->m_width && src->m_height == dst->m_height && "Sizes of the textures are not the same.");

    if (m_copyFb == nullptr)
    {
      FramebufferSettings fbSettings = {src->m_width, src->m_height, false, false};
      m_copyFb                       = MakeNewPtr<Framebuffer>(fbSettings, "RendererCopyFB");
      m_copyFb->Init();
    }

    m_copyFb->ReconstructIfNeeded(src->m_width, src->m_height);

    FramebufferPtr lastFb = m_framebuffer;

    RenderTargetPtr rt    = Cast<RenderTarget>(dst);
    m_copyFb->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, rt);
    SetFramebuffer(m_copyFb, GraphicBitFields::AllBits);

    // Render to texture
    if (m_copyMaterial == nullptr)
    {
      m_copyMaterial                   = MakeNewPtr<Material>();
      m_copyMaterial->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("copyTextureVert.shader", true));
      m_copyMaterial->m_fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("copyTextureFrag.shader", true));
    }

    m_copyMaterial->m_diffuseTexture = src;
    m_copyMaterial->Init();

    DrawFullQuad(m_copyMaterial);
    SetFramebuffer(lastFb, GraphicBitFields::None);
  }

  void Renderer::OverrideBlendState(bool enableOverride, BlendFunction func)
  {
    RenderState stateCpy       = m_renderState;
    stateCpy.blendFunction     = func;

    m_blendStateOverrideEnable = false;

    SetRenderState(&stateCpy);

    m_blendStateOverrideEnable = enableOverride;
  }

  void Renderer::EnableBlending(bool enable)
  {
    if (enable)
    {
      glEnable(GL_BLEND);
    }
    else
    {
      glDisable(GL_BLEND);
    }
  }

  void Renderer::EnableDepthWrite(bool enable) { glDepthMask(enable); }

  void Renderer::EnableDepthTest(bool enable)
  {
    if (m_renderState.depthTestEnabled != enable)
    {
      if (enable)
      {
        glEnable(GL_DEPTH_TEST);
      }
      else
      {
        glDisable(GL_DEPTH_TEST);
      }
      m_renderState.depthTestEnabled = enable;
    }
  }

  void Renderer::SetDepthTestFunc(CompareFunctions func)
  {
    if (m_renderState.depthFunction != func)
    {
      m_renderState.depthFunction = func;
      glDepthFunc((int) func);
    }
  }

  void Renderer::Apply7x1GaussianBlur(const TexturePtr src, RenderTargetPtr dst, const Vec3& axis, const float amount)
  {
    m_oneColorAttachmentFramebuffer->ReconstructIfNeeded({dst->m_width, dst->m_height, false, false});

    if (m_gaussianBlurMaterial == nullptr)
    {
      ShaderPtr vert         = GetShaderManager()->Create<Shader>(ShaderPath("gausBlur7x1Vert.shader", true));
      ShaderPtr frag         = GetShaderManager()->Create<Shader>(ShaderPath("gausBlur7x1Frag.shader", true));

      m_gaussianBlurMaterial = MakeNewPtr<Material>();
      m_gaussianBlurMaterial->m_vertexShader   = vert;
      m_gaussianBlurMaterial->m_fragmentShader = frag;
      m_gaussianBlurMaterial->m_diffuseTexture = nullptr;
      m_gaussianBlurMaterial->Init();
    }

    m_gaussianBlurMaterial->m_diffuseTexture = src;
    m_gaussianBlurMaterial->UpdateProgramUniform("BlurScale", axis * amount);

    m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, dst);

    SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
    DrawFullQuad(m_gaussianBlurMaterial);
  }

  void Renderer::ApplyAverageBlur(const TexturePtr src, RenderTargetPtr dst, const Vec3& axis, const float amount)
  {
    m_oneColorAttachmentFramebuffer->ReconstructIfNeeded({dst->m_width, dst->m_height, false, false});

    if (m_averageBlurMaterial == nullptr)
    {
      ShaderPtr vert        = GetShaderManager()->Create<Shader>(ShaderPath("avgBlurVert.shader", true));
      ShaderPtr frag        = GetShaderManager()->Create<Shader>(ShaderPath("avgBlurFrag.shader", true));

      m_averageBlurMaterial = MakeNewPtr<Material>();
      m_averageBlurMaterial->m_vertexShader   = vert;
      m_averageBlurMaterial->m_fragmentShader = frag;
      m_averageBlurMaterial->m_diffuseTexture = nullptr;
      m_averageBlurMaterial->Init();
    }

    m_averageBlurMaterial->m_diffuseTexture = src;
    m_averageBlurMaterial->UpdateProgramUniform("BlurScale", axis * amount);

    m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, dst);

    SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
    DrawFullQuad(m_averageBlurMaterial);
  }

  void Renderer::GenerateBRDFLutTexture()
  {
    if (!GetTextureManager()->Exist(TKBrdfLutTexture))
    {
      FramebufferPtr prevFrameBuffer = GetFrameBuffer();

      TextureSettings set;
      set.InternalFormat = GraphicTypes::FormatRG16F;
      set.Format         = GraphicTypes::FormatRG;
      set.Type           = GraphicTypes::TypeFloat;
      set.GenerateMipMap = false;

      RenderTargetPtr brdfLut =
          MakeNewPtr<RenderTarget>(RHIConstants::BrdfLutTextureSize, RHIConstants::BrdfLutTextureSize, set);
      brdfLut->Init();

      FramebufferSettings fbSettings = {RHIConstants::BrdfLutTextureSize,
                                        RHIConstants::BrdfLutTextureSize,
                                        false,
                                        false};

      FramebufferPtr utilFramebuffer = MakeNewPtr<Framebuffer>(fbSettings, "RendererLUTFB");
      utilFramebuffer->Init();
      utilFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, brdfLut);

      MaterialPtr material       = MakeNewPtr<Material>();
      material->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("fullQuadVert.shader", true));
      material->m_fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("BRDFLutFrag.shader", true));

      QuadPtr quad               = MakeNewPtr<Quad>();
      MeshPtr mesh               = quad->GetMeshComponent()->GetMeshVal();
      mesh->m_material           = material;
      mesh->Init();

      SetFramebuffer(utilFramebuffer, GraphicBitFields::AllBits);

      CameraPtr camera = MakeNewPtr<Camera>();
      SetCamera(camera, true);

      RenderJobArray jobs;
      RenderJobProcessor::CreateRenderJobs(jobs, quad);
      RenderWithProgramFromMaterial(jobs);

      brdfLut->SetFile(TKBrdfLutTexture);
      GetTextureManager()->Manage(brdfLut);
      m_brdfLut = brdfLut;

      SetFramebuffer(prevFrameBuffer, GraphicBitFields::None);
    }
  }

  void Renderer::SetAmbientOcclusionTexture(TexturePtr aoTexture)
  {
    m_aoTexture = aoTexture;
    if (m_aoTexture)
    {
      SetTexture(5, m_aoTexture->m_textureId);
    }
  }

  void Renderer::BindProgramOfMaterial(Material* material)
  {
    material->Init();
    GpuProgramPtr program = m_gpuProgramManager->CreateProgram(material->m_vertexShader, material->m_fragmentShader);
    BindProgram(program);
  }

  void Renderer::BindProgram(const GpuProgramPtr& program)
  {
    if (m_currentProgram == nullptr || m_currentProgram->m_handle != program->m_handle)
    {
      m_currentProgram = program;
      glUseProgram(program->m_handle);
    }
  }

  void Renderer::ResetUsedTextureSlots()
  {
    for (int i = 0; i < 17; i++)
    {
      SetTexture(i, 0);
    }
  }

  void Renderer::FeedUniforms(const GpuProgramPtr& program, const RenderJob& job)
  {
    // Update camera related uniforms.
    if (m_gpuProgramHasCameraUpdates.find(program->m_handle) == m_gpuProgramHasCameraUpdates.end())
    {
      int uniformLoc = program->GetDefaultUniformLocation(Uniform::VIEW);
      if (uniformLoc != -1)
      {
        glUniformMatrix4fv(uniformLoc, 1, false, &m_view[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::PROJECT_VIEW_NO_TR);
      if (uniformLoc != -1)
      {
        glUniformMatrix4fv(uniformLoc, 1, false, &m_projectViewNoTranslate[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::CAM_DATA_POS);
      if (uniformLoc != -1)
      {
        glUniform3fv(uniformLoc, 1, &m_camPos.x);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::CAM_DATA_DIR);
      if (uniformLoc != -1)
      {
        glUniform3fv(uniformLoc, 1, &m_camDirection.x);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::CAM_DATA_FAR);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, m_camFar);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::SHADOW_DISTANCE);
      if (uniformLoc != -1)
      {
        EngineSettings& set = GetEngineSettings();
        glUniform1f(uniformLoc, set.Graphics.GetShadowMaxDistance());
      }

      m_gpuProgramHasCameraUpdates.insert(program->m_handle);
    }

    // Update Per frame changing uniforms. ExecuteRenderTasks clears programs at the beginning of the call.
    if (m_gpuProgramHasFrameUpdates.find(program->m_handle) == m_gpuProgramHasFrameUpdates.end())
    {
      int uniformLoc = program->GetDefaultUniformLocation(Uniform::FRAME_COUNT);
      if (uniformLoc != -1)
      {
        glUniform1ui(uniformLoc, m_frameCount);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::ELAPSED_TIME);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, Main::GetInstance()->TimeSinceStartup() / 1000.0f);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::SHADOW_ATLAS_SIZE);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, (float) RHIConstants::ShadowAtlasTextureSize);
      }

      m_gpuProgramHasFrameUpdates.insert(program->m_handle);
    }

    bool updateMaterial = false;
    if (program->m_activeMaterialID == m_mat->GetIdVal())
    {
      updateMaterial = program->m_activeMaterialVersion != m_mat->GetRuntimeVersion();
    }
    else
    {
      updateMaterial = true;
    }

    int uniformLoc = -1;
    if (updateMaterial)
    {
      program->m_activeMaterialID      = m_mat->GetIdVal();
      program->m_activeMaterialVersion = m_mat->GetRuntimeVersion();

      uniformLoc                       = program->GetDefaultUniformLocation(Uniform::COLOR);
      if (uniformLoc != -1)
      {
        Vec4 color = Vec4(m_mat->m_color, m_mat->GetAlpha());
        if (m_mat->GetRenderState()->blendFunction == BlendFunction::NONE)
        {
          color.w = 1.0f;
        }

        if (m_renderOnlyLighting)
        {
          color = Vec4(1.0f, 1.0f, 1.0f, color.w);
        }
        glUniform4fv(uniformLoc, 1, &color.x);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::COLOR_ALPHA);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, m_mat->GetAlpha());
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::USE_ALPHA_MASK);
      if (uniformLoc != -1)
      {
        glUniform1i(uniformLoc, m_renderState.blendFunction == BlendFunction::ALPHA_MASK);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::ALPHA_MASK_TRESHOLD);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, m_renderState.alphaMaskTreshold);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::DIFFUSE_TEXTURE_IN_USE);
      if (uniformLoc != -1)
      {
        int diffInUse = (int) (m_mat->m_diffuseTexture != nullptr);
        if (m_renderOnlyLighting)
        {
          diffInUse = false;
        }
        glUniform1i(uniformLoc, diffInUse);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::EMISSIVE_TEXTURE_IN_USE);
      if (uniformLoc != -1)
      {
        int emmInUse = (int) (m_mat->m_emissiveTexture != nullptr);
        glUniform1i(uniformLoc, emmInUse);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::EMISSIVE_COLOR);
      if (uniformLoc != -1)
      {
        glUniform3fv(uniformLoc, 1, &m_mat->m_emissiveColor.x);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::NORMAL_MAP_IN_USE);
      if (uniformLoc != -1)
      {
        int normInUse = (int) (m_mat->m_normalMap != nullptr);
        glUniform1i(uniformLoc, normInUse);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE);
      if (uniformLoc != -1)
      {
        int metRghInUse = (int) (m_mat->m_metallicRoughnessTexture != nullptr);
        glUniform1i(uniformLoc, metRghInUse);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::METALLIC);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, (GLfloat) m_mat->m_metallic);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::ROUGHNESS);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, (GLfloat) m_mat->m_roughness);
      }
    }

    // Built-in variables.
    {
      uniformLoc = program->GetDefaultUniformLocation(Uniform::MODEL_VIEW_MATRIX);
      if (uniformLoc != -1)
      {
        const Mat4 modelView = m_view * m_model;
        glUniformMatrix4fv(uniformLoc, 1, false, &modelView[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::PROJECT_VIEW_MODEL);
      if (uniformLoc != -1)
      {
        Mat4 mul = m_projectView * m_model;
        glUniformMatrix4fv(uniformLoc, 1, false, &mul[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::MODEL);
      if (uniformLoc != -1)
      {
        glUniformMatrix4fv(uniformLoc, 1, false, &m_model[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::MODEL_NO_TR);
      if (uniformLoc != -1)
      {
        Mat4 modelNoTr  = m_model;
        modelNoTr[0][3] = 0.0f;
        modelNoTr[1][3] = 0.0f;
        modelNoTr[2][3] = 0.0f;
        modelNoTr[3][3] = 1.0f;
        modelNoTr[3][0] = 0.0f;
        modelNoTr[3][1] = 0.0f;
        modelNoTr[3][2] = 0.0f;
        glUniformMatrix4fv(uniformLoc, 1, false, &modelNoTr[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::INV_TR_MODEL);
      if (uniformLoc != -1)
      {
        Mat4 invTrModel = glm::transpose(glm::inverse(m_model));
        glUniformMatrix4fv(uniformLoc, 1, false, &invTrModel[0][0]);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE);
      if (uniformLoc != -1)
      {
        if (m_mat->m_metallicRoughnessTexture != nullptr)
        {
          if (m_mat->m_metallicRoughnessTexture)
          {
            SetTexture(4, m_mat->m_metallicRoughnessTexture->m_textureId);
          }
        }
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::NORMAL_MAP_IN_USE);
      if (uniformLoc != -1)
      {
        if (m_mat->m_normalMap != nullptr)
        {
          SetTexture(9, m_mat->m_normalMap->m_textureId);
        }
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::USE_IBL);
      if (uniformLoc != -1)
      {
        if (m_renderState.IBLInUse)
        {
          SetTexture(7, m_renderState.irradianceMap);
          SetTexture(15, m_renderState.preFilteredSpecularMap);
          SetTexture(16, m_renderState.brdfLut);
        }
        glUniform1i(uniformLoc, (GLint) m_renderState.IBLInUse);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::IBL_ROTATION);
      if (uniformLoc != -1)
      {
        if (m_renderState.IBLInUse)
        {
          glUniformMatrix4fv(uniformLoc, 1, false, &m_iblRotation[0][0]);
        }
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::IBL_INTENSITY);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, m_renderState.iblIntensity);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::IBL_MAX_REFLECTION_LOD);
      if (uniformLoc != -1)
      {
        glUniform1i(uniformLoc, RHIConstants::SpecularIBLLods - 1);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::EMISSIVE_TEXTURE_IN_USE);
      if (uniformLoc != -1)
      {
        if (m_mat->m_emissiveTexture != nullptr)
        {
          SetTexture(1, m_mat->m_emissiveTexture->m_textureId);
        }
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::AO_ENABLED);
      if (uniformLoc != -1)
      {
        bool aoEnabled = m_aoTexture != nullptr;
        glUniform1i(uniformLoc, aoEnabled);
      }
    }

    for (auto& uniform : program->m_customUniforms)
    {
      GLint loc = program->GetCustomUniformLocation(uniform.second);

      // custom defined variable
      switch (uniform.second.GetType())
      {
      case ShaderUniform::UniformType::Bool:
        glUniform1ui(loc, uniform.second.GetVal<bool>());
        break;
      case ShaderUniform::UniformType::Float:
        glUniform1f(loc, uniform.second.GetVal<float>());
        break;
      case ShaderUniform::UniformType::Int:
        glUniform1i(loc, uniform.second.GetVal<int>());
        break;
      case ShaderUniform::UniformType::UInt:
        glUniform1ui(loc, uniform.second.GetVal<uint>());
        break;
      case ShaderUniform::UniformType::Vec2:
        glUniform2fv(loc, 1, reinterpret_cast<float*>(&uniform.second.GetVal<Vec2>()));
        break;
      case ShaderUniform::UniformType::Vec3:
        glUniform3fv(loc, 1, reinterpret_cast<float*>(&uniform.second.GetVal<Vec3>()));
        break;
      case ShaderUniform::UniformType::Vec4:
        glUniform4fv(loc, 1, reinterpret_cast<float*>(&uniform.second.GetVal<Vec4>()));
        break;
      case ShaderUniform::UniformType::Mat3:
        glUniformMatrix3fv(loc, 1, false, reinterpret_cast<float*>(&uniform.second.GetVal<Mat3>()));
        break;
      case ShaderUniform::UniformType::Mat4:
        glUniformMatrix4fv(loc, 1, false, reinterpret_cast<float*>(&uniform.second.GetVal<Mat4>()));
        break;
      default:
        assert(false && "Invalid type.");
        break;
      }
    }
  }

  void Renderer::FeedLightUniforms(const GpuProgramPtr& program, const RenderJob& job)
  {
    GLint loc = program->GetDefaultUniformLocation(Uniform::LIGHT_DATA_ACTIVECOUNT);
    if (loc != -1)
    {
      glUniform1i(loc, (int) job.lights.size());
    }

    if (job.lights.empty())
    {
      return;
    }

    m_lightCache.SetDrawCallVersion(m_drawCallVersion);

    if (GetRenderSystem()->ConsumeGPULightCacheInvalidation())
    {
      m_lightCache.UpdateVersion(); // This will cause an update of the cache.
    }

    // Make sure the cache has the lights that is going to rendered
    for (uint i = 0; i < job.lights.size(); ++i)
    {
      Light* light             = job.lights[i];
      light->m_drawCallVersion = m_drawCallVersion;

      int indexInCache         = m_lightCache.Contains(light);
      if (indexInCache != -1)
      {
        // If light is invalidated or cache index has changed, invalidate the light cache.
        if (light->m_invalidatedForLightCache || light->m_lightCacheIndex != indexInCache)
        {
          light->m_lightCacheIndex          = indexInCache;
          light->m_invalidatedForLightCache = false;
          m_lightCache.UpdateVersion(); // Light needs to be updated, invalidate the cache.
        }
      }
      else
      {
        light->m_lightCacheIndex = m_lightCache.Add(light); // Add will cause a cache invalidation.
      }
    }

    m_drawCallVersion++;

    // When cache is invalidated, update the cache for this program
    if (program->m_lightCacheVersion != m_lightCache.GetVersion())
    {
      // Update the cache.
      program->m_lightCacheVersion = m_lightCache.GetVersion();
      m_lightDataBuffer.UpdateLightCache(m_lightCache.GetLights(), RHIConstants::LightCacheSize);
      m_lightDataBuffer.UpdateActiveLights(job.lights, true);
    }

    // Checks the current active light indexes, perform the update only if necessary.
    m_lightDataBuffer.UpdateActiveLights(job.lights);

    // Bind shadow map if activated
    if (m_shadowAtlas != nullptr)
    {
      SetTexture(8, m_shadowAtlas->m_textureId);
    }
  }

  void Renderer::FeedAnimationUniforms(const GpuProgramPtr& program, const RenderJob& job)
  {
    // Send if its animated or not.
    int uniformLoc = program->GetDefaultUniformLocation(Uniform::IS_ANIMATED);
    if (uniformLoc != -1)
    {
      glUniform1ui(uniformLoc, job.animData.currentAnimation != nullptr);
    }

    if (job.animData.currentAnimation == nullptr)
    {
      // If not animated, just skip the rest.
      return;
    }

    // Send key frames.
    uniformLoc = program->GetDefaultUniformLocation(Uniform::KEY_FRAME_COUNT);
    if (uniformLoc != -1)
    {
      glUniform1f(uniformLoc, job.animData.keyFrameCount);
    }

    if (job.animData.keyFrameCount > 0)
    {
      uniformLoc = program->GetDefaultUniformLocation(Uniform::KEY_FRAME_1);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.firstKeyFrame);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::KEY_FRAME_2);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.secondKeyFrame);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::KEY_FRAME_INT_TIME);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.keyFrameInterpolationTime);
      }
    }

    // Send blend data.
    uniformLoc = program->GetDefaultUniformLocation(Uniform::BLEND_ANIMATION);
    if (uniformLoc != -1)
    {
      glUniform1i(uniformLoc, job.animData.blendAnimation != nullptr);
    }

    if (job.animData.blendAnimation != nullptr)
    {
      uniformLoc = program->GetDefaultUniformLocation(Uniform::BLEND_FACTOR);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.animationBlendFactor);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::BLEND_KEY_FRAME_1);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.blendFirstKeyFrame);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::BLEND_KEY_FRAME_2);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.blendSecondKeyFrame);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::BLEND_KEY_FRAME_INT_TIME);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.blendKeyFrameInterpolationTime);
      }

      uniformLoc = program->GetDefaultUniformLocation(Uniform::BLEND_KEY_FRAME_COUNT);
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, job.animData.blendKeyFrameCount);
      }
    }
  }

  void Renderer::SetTexture(ubyte slotIndx, uint textureId)
  {
    assert(slotIndx < 17 && "You exceed texture slot count");

    static const GLenum textureTypeLut[17] = {
        GL_TEXTURE_2D,       // 0 -> Color Texture
        GL_TEXTURE_2D,       // 1 -> Emissive Texture
        GL_TEXTURE_2D,       // 2 -> EMPTY
        GL_TEXTURE_2D,       // 3 -> Skinning informatison
        GL_TEXTURE_2D,       // 4 -> Metallic Roughness Texture
        GL_TEXTURE_2D,       // 5 -> AO Texture
        GL_TEXTURE_CUBE_MAP, // 6 -> Cubemap
        GL_TEXTURE_CUBE_MAP, // 7 -> Irradiance Map
        GL_TEXTURE_2D_ARRAY, // 8 -> Shadow Atlas
        GL_TEXTURE_2D,       // 9 -> Normal map, gbuffer position
        GL_TEXTURE_2D,       // 10 -> gBuffer normal texture
        GL_TEXTURE_2D,       // 11 -> gBuffer color texture
        GL_TEXTURE_2D,       // 12 -> gBuffer emissive texture
        GL_TEXTURE_2D,       // 13 -> EMPTY
        GL_TEXTURE_2D,       // 14 -> gBuffer metallic roughness texture
        GL_TEXTURE_CUBE_MAP, // 15 -> IBL Specular Pre-Filtered Map
        GL_TEXTURE_2D        // 16 -> IBL BRDF Lut
    };

    RHI::SetTexture(textureTypeLut[slotIndx], textureId, slotIndx);
  }

  void Renderer::SetShadowAtlas(TexturePtr shadowAtlas) { m_shadowAtlas = shadowAtlas; }

  CubeMapPtr Renderer::GenerateCubemapFrom2DTexture(TexturePtr texture, uint size, float exposure)
  {
    const TextureSettings set = {GraphicTypes::TargetCubeMap,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::SampleLinear,
                                 GraphicTypes::SampleLinear,
                                 GraphicTypes::FormatRGBA16F,
                                 GraphicTypes::FormatRGBA,
                                 GraphicTypes::TypeFloat,
                                 1,
                                 false};

    RenderTargetPtr cubeMapRt = MakeNewPtr<RenderTarget>(size, size, set);
    cubeMapRt->Init();

    // Create material
    MaterialPtr mat                 = MakeNewPtr<Material>();
    ShaderPtr vert                  = GetShaderManager()->Create<Shader>(ShaderPath("equirectToCubeVert.shader", true));
    ShaderPtr frag                  = GetShaderManager()->Create<Shader>(ShaderPath("equirectToCubeFrag.shader", true));

    mat->m_diffuseTexture           = texture;
    mat->m_vertexShader             = vert;
    mat->m_fragmentShader           = frag;
    mat->GetRenderState()->cullMode = CullingType::TwoSided;
    mat->Init();

    mat->UpdateProgramUniform("Exposure", exposure);

    m_oneColorAttachmentFramebuffer->ReconstructIfNeeded({(int) size, (int) size, false, false});

    // Views for 6 different angles
    CameraPtr cam = MakeNewPtr<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; i++)
    {
      Vec3 pos, sca;
      Quaternion rot;

      DecomposeMatrix(views[i], &pos, &rot, &sca);

      cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
      cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
      cam->m_node->SetScale(sca);

      m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                                          cubeMapRt,
                                                          0,
                                                          -1,
                                                          (Framebuffer::CubemapFace) i);

      SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
      DrawCube(cam, mat);
    }

    CubeMapPtr cubeMap = MakeNewPtr<CubeMap>();
    cubeMap->Consume(cubeMapRt);

    return cubeMap;
  }

  CubeMapPtr Renderer::GenerateDiffuseEnvMap(CubeMapPtr cubemap, uint size)
  {
    const TextureSettings set = {GraphicTypes::TargetCubeMap,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::SampleLinear,
                                 GraphicTypes::SampleLinear,
                                 GraphicTypes::FormatRGBA16F,
                                 GraphicTypes::FormatRGBA,
                                 GraphicTypes::TypeFloat,
                                 1,
                                 false};

    RenderTargetPtr cubeMapRt = MakeNewPtr<RenderTarget>(size, size, set);
    cubeMapRt->Init();

    // Views for 6 different angles
    CameraPtr cam = MakeNewPtr<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[]          = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                             glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    // Create material
    MaterialPtr mat       = MakeNewPtr<Material>();
    ShaderPtr vert        = GetShaderManager()->Create<Shader>(ShaderPath("irradianceGenerateVert.shader", true));
    ShaderPtr frag        = GetShaderManager()->Create<Shader>(ShaderPath("irradianceGenerateFrag.shader", true));

    mat->m_cubeMap        = cubemap;
    mat->m_vertexShader   = vert;
    mat->m_fragmentShader = frag;
    mat->GetRenderState()->cullMode = CullingType::TwoSided;
    mat->Init();

    m_oneColorAttachmentFramebuffer->ReconstructIfNeeded({(int) size, (int) size, false, false});

    for (int i = 0; i < 6; ++i)
    {
      Vec3 pos;
      Quaternion rot;
      Vec3 sca;
      DecomposeMatrix(views[i], &pos, &rot, &sca);

      cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
      cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
      cam->m_node->SetScale(sca);

      m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                                          cubeMapRt,
                                                          0,
                                                          -1,
                                                          (Framebuffer::CubemapFace) i);

      SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
      DrawCube(cam, mat);
    }

    SetFramebuffer(nullptr, GraphicBitFields::None);

    CubeMapPtr newCubeMap = MakeNewPtr<CubeMap>();
    newCubeMap->Consume(cubeMapRt);

    return newCubeMap;
  }

  CubeMapPtr Renderer::GenerateSpecularEnvMap(CubeMapPtr cubemap, uint size, int mipMaps)
  {
    const TextureSettings set = {GraphicTypes::TargetCubeMap,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::UVClampToEdge,
                                 GraphicTypes::SampleNearest,
                                 GraphicTypes::SampleNearest,
                                 GraphicTypes::FormatRGBA16F,
                                 GraphicTypes::FormatRGBA,
                                 GraphicTypes::TypeFloat,
                                 1,
                                 false};

    RenderTargetPtr cubemapRt = MakeNewPtr<RenderTarget>(size, size, set);
    cubemapRt->Init();

    // Intentionally creating space to fill later. ( mip maps will be calculated for specular ibl )
    RHI::SetTexture(GL_TEXTURE_CUBE_MAP, cubemapRt->m_textureId, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Views for 6 different angles
    CameraPtr cam = MakeNewPtr<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[]          = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                             glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                             glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    // Create material
    MaterialPtr mat       = MakeNewPtr<Material>();
    ShaderPtr vert        = GetShaderManager()->Create<Shader>(ShaderPath("positionVert.shader", true));
    ShaderPtr frag        = GetShaderManager()->Create<Shader>(ShaderPath("preFilterEnvMapFrag.shader", true));

    mat->m_cubeMap        = cubemap;
    mat->m_vertexShader   = vert;
    mat->m_fragmentShader = frag;
    mat->GetRenderState()->cullMode = CullingType::TwoSided;
    mat->Init();

    m_oneColorAttachmentFramebuffer->ReconstructIfNeeded({(int) size, (int) size, false, false});

    UVec2 lastViewportSize = m_viewportSize;

    assert(size >= 128 && "Due to RHIConstants::SpecularIBLLods, it can't be lower than this resolution.");
    for (int mip = 0; mip < mipMaps; mip++)
    {
      uint mipSize              = (uint) (size * std::powf(0.5f, (float) mip));

      // Create a temporary cubemap for each mipmap level
      RenderTargetPtr mipCubeRt = MakeNewPtr<RenderTarget>(mipSize, mipSize, set);
      mipCubeRt->Init();

      for (int i = 0; i < 6; ++i)
      {
        Vec3 pos;
        Quaternion rot;
        Vec3 sca;
        DecomposeMatrix(views[i], &pos, &rot, &sca);

        cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
        cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
        cam->m_node->SetScale(sca);

        m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0,
                                                            mipCubeRt,
                                                            0,
                                                            -1,
                                                            (Framebuffer::CubemapFace) i);

        SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);

        mat->UpdateProgramUniform("roughness", (float) mip / (float) mipMaps);
        mat->UpdateProgramUniform("resPerFace", (float) mipSize);

        RHI::SetTexture(GL_TEXTURE_CUBE_MAP, cubemap->m_textureId, 0);

        DrawCube(cam, mat);

        // Copy color attachment to cubemap's correct mip level and face.
        RHI::SetTexture(GL_TEXTURE_CUBE_MAP, cubemapRt->m_textureId, 0);
        glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, 0, 0, 0, 0, mipSize, mipSize);
      }
    }

    SetFramebuffer(nullptr, GraphicBitFields::None);

    CubeMapPtr newCubeMap = MakeNewPtr<CubeMap>();
    newCubeMap->Consume(cubemapRt);

    return newCubeMap;
  }

} // namespace ToolKit
