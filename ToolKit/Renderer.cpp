/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Renderer.h"

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
#include "ResourceComponent.h"
#include "Scene.h"
#include "Shader.h"
#include "Skeleton.h"
#include "Surface.h"
#include "TKAssert.h"
#include "TKOpenGL.h"
#include "TKProfiler.h"
#include "TKStats.h"
#include "Texture.h"
#include "ToolKit.h"
#include "UIManager.h"
#include "Viewport.h"

#include "DebugNew.h"

namespace ToolKit
{
  Renderer::Renderer() {}

  void Renderer::Init()
  {
    m_uiCamera                      = MakeNewPtr<Camera>();
    m_oneColorAttachmentFramebuffer = MakeNewPtr<Framebuffer>();
    m_dummyDrawCube                 = MakeNewPtr<Cube>();

    m_gpuProgramManager             = GetGpuProgramManager();
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
  }

  void Renderer::SetLights(const LightPtrArray& lights) { m_lights = lights; }

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
    assert(m_ignoreRenderingCulledObjectWarning || !job.frustumCulled && "Rendering culled object.");

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
        // animation
        AnimationPlayer* animPlayer = GetAnimationPlayer();
        SetTexture(3,
                   animPlayer->GetAnimationDataTexture(skel->GetIdVal(), job.animData.currentAnimation->GetIdVal())
                       ->m_textureId);

        // animation to blend
        if (job.animData.blendAnimation != nullptr)
        {
          SetTexture(2,
                     animPlayer->GetAnimationDataTexture(skel->GetIdVal(), job.animData.blendAnimation->GetIdVal())
                         ->m_textureId);
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
    m_mat = m_overrideMat != nullptr ? m_overrideMat.get() : job.Material;
    m_mat->Init();

    RenderState* rs = m_mat->GetRenderState();
    SetRenderState(rs);

    auto activateSkinning = [&](const Mesh* mesh)
    {
      GLint isSkinnedLoc = m_currentProgram->GetUniformLocation(GetUniformName(Uniform::IS_SKINNED));
      bool isSkinned     = mesh->IsSkinned();
      if (isSkinned)
      {
        SkeletonPtr skel = static_cast<SkinMesh*>(job.Mesh)->m_skeleton;
        assert(skel != nullptr);

        GLint numBonesLoc = m_currentProgram->GetUniformLocation(GetUniformName(Uniform::NUM_BONES));
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

    FeedLightUniforms(m_currentProgram, job);
    FeedUniforms(m_currentProgram, job);

    RHI::BindVertexArray(mesh->m_vaoId);

    if (mesh->m_indexCount != 0)
    {
      glDrawElements((GLenum) rs->drawType, mesh->m_indexCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
      glDrawArrays((GLenum) rs->drawType, 0, mesh->m_vertexCount);
    }

    AddDrawCall();

    m_overrideMat = nullptr;
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
    MaterialPtr overrideMaterial = m_overrideMat;

    for (const RenderJob& job : jobs)
    {
      if (overrideMaterial != nullptr)
      {
        m_overrideMat = overrideMaterial;
      }

      Render(job);
    }
  }

  void Renderer::SetRenderState(const RenderState* const state)
  {
    CPU_FUNC_RANGE();

    if (m_renderState.cullMode != state->cullMode)
    {
      if (state->cullMode == CullingType::TwoSided)
      {
        glDisable(GL_CULL_FACE);
      }

      if (state->cullMode == CullingType::Front)
      {
        if (m_renderState.cullMode == CullingType::TwoSided)
        {
          glEnable(GL_CULL_FACE);
        }
        glCullFace(GL_FRONT);
      }

      if (state->cullMode == CullingType::Back)
      {
        if (m_renderState.cullMode == CullingType::TwoSided)
        {
          glEnable(GL_CULL_FACE);
        }
        glCullFace(GL_BACK);
      }

      m_renderState.cullMode = state->cullMode;
    }

    if (m_renderState.blendFunction != state->blendFunction)
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
    CPU_FUNC_RANGE();

    if (fb != m_framebuffer)
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
    }

    if (attachmentsToClear != GraphicBitFields::None)
    {
      ClearBuffer(attachmentsToClear, clearColor);
    }

    m_framebuffer = fb;
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

    RHI::SetFramebuffer(GL_READ_FRAMEBUFFER, srcId);
    RHI::SetFramebuffer(GL_DRAW_FRAMEBUFFER, dest->GetFboId());

    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, (GLbitfield) fields, GL_NEAREST);
  }

  void Renderer::InvalidateFramebufferDepth(FramebufferPtr fb)
  {
    constexpr GLenum invalidAttachments[1] = {GL_DEPTH_ATTACHMENT};

    SetFramebuffer(fb, GraphicBitFields::None);
    RHI::InvalidateFramebuffer(fb->GetFboId(), 1, invalidAttachments);
  }

  void Renderer::InvalidateFramebufferStencil(FramebufferPtr fb)
  {
    constexpr GLenum invalidAttachments[1] = {GL_STENCIL_ATTACHMENT};

    SetFramebuffer(fb, GraphicBitFields::None);
    RHI::InvalidateFramebuffer(fb->GetFboId(), 1, invalidAttachments);
  }

  void Renderer::InvalidateFramebufferDepthStencil(FramebufferPtr fb)
  {
    constexpr GLenum invalidAttachments[2] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};

    SetFramebuffer(fb, GraphicBitFields::None);
    RHI::InvalidateFramebuffer(fb->GetFboId(), 2, invalidAttachments);
  }

  void Renderer::SetViewport(Viewport* viewport) { SetFramebuffer(viewport->m_framebuffer, GraphicBitFields::AllBits); }

  void Renderer::SetViewportSize(uint width, uint height)
  {
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
    MaterialPtr material   = MakeNewPtr<Material>();
    material->UnInit();

    material->m_vertexShader   = fullQuadVert;
    material->m_fragmentShader = fragmentShader;
    material->Init();

    DrawFullQuad(material);
  }

  void Renderer::DrawFullQuad(MaterialPtr mat)
  {
    QuadPtr quad                                       = MakeNewPtr<Quad>();
    quad->GetMeshComponent()->GetMeshVal()->m_material = mat;

    CameraPtr quadCam                                  = MakeNewPtr<Camera>();
    SetCamera(quadCam, true);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJobs({quad}, jobs);

    bool lastDepthTestState = m_renderState.depthTestEnabled;
    EnableDepthTest(false);

    RenderWithProgramFromMaterial(jobs);

    EnableDepthTest(lastDepthTestState);
  }

  void Renderer::DrawCube(CameraPtr cam, MaterialPtr mat, const Mat4& transform)
  {
    m_dummyDrawCube->m_node->SetTransform(transform);
    m_dummyDrawCube->GetMaterialComponent()->SetFirstMaterial(mat);
    SetCamera(cam, true);

    RenderJobArray jobs;
    EntityPtrArray oneDummyDrawCube = {m_dummyDrawCube};
    RenderJobProcessor::CreateRenderJobs(oneDummyDrawCube, jobs);
    RenderWithProgramFromMaterial(jobs);
  }

  void Renderer::CopyTexture(TexturePtr source, TexturePtr dest)
  {
    CPU_FUNC_RANGE();

    assert(source->m_width == dest->m_width && source->m_height == dest->m_height &&
           "Sizes of the textures are not the same.");

    assert(source->m_initiated && dest->m_initiated && "Texture is not initialized.");

    assert(source);

    if (m_copyFb == nullptr)
    {
      m_copyFb = MakeNewPtr<Framebuffer>();
      m_copyFb->Init({source->m_width, source->m_height, false, false});
    }

    RenderTargetPtr rt = std::static_pointer_cast<RenderTarget>(dest);
    m_copyFb->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, rt);

    // Set and clear fb
    FramebufferPtr lastFb = m_framebuffer;
    SetFramebuffer(m_copyFb, GraphicBitFields::None);

    // Render to texture
    if (m_copyMaterial == nullptr)
    {
      m_copyMaterial                   = MakeNewPtr<Material>();
      m_copyMaterial->m_vertexShader   = GetShaderManager()->Create<Shader>(ShaderPath("copyTextureVert.shader", true));
      m_copyMaterial->m_fragmentShader = GetShaderManager()->Create<Shader>(ShaderPath("copyTextureFrag.shader", true));
    }

    m_copyMaterial->UnInit();
    m_copyMaterial->m_diffuseTexture = source;
    m_copyMaterial->Init();

    DrawFullQuad(m_copyMaterial);
    SetFramebuffer(lastFb, GraphicBitFields::None);
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

  void Renderer::Apply7x1GaussianBlur(const TexturePtr source,
                                      RenderTargetPtr dest,
                                      const Vec3& axis,
                                      const float amount)
  {
    CPU_FUNC_RANGE();

    FramebufferPtr frmBackup = m_framebuffer;

    m_oneColorAttachmentFramebuffer->Init({0, 0, false, false});

    if (m_gaussianBlurMaterial == nullptr)
    {
      ShaderPtr vert         = GetShaderManager()->Create<Shader>(ShaderPath("gausBlur7x1Vert.shader", true));
      ShaderPtr frag         = GetShaderManager()->Create<Shader>(ShaderPath("gausBlur7x1Frag.shader", true));

      m_gaussianBlurMaterial = MakeNewPtr<Material>();
      m_gaussianBlurMaterial->m_vertexShader   = vert;
      m_gaussianBlurMaterial->m_fragmentShader = frag;
      m_gaussianBlurMaterial->m_diffuseTexture = nullptr;
    }

    m_gaussianBlurMaterial->UnInit();
    m_gaussianBlurMaterial->m_diffuseTexture = source;
    m_gaussianBlurMaterial->Init();
    m_gaussianBlurMaterial->UpdateUniformOfThisMaterialsProgram("BlurScale", axis * amount);

    m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, dest);

    SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
    DrawFullQuad(m_gaussianBlurMaterial);

    SetFramebuffer(frmBackup, GraphicBitFields::None);
  }

  void Renderer::ApplyAverageBlur(const TexturePtr source, RenderTargetPtr dest, const Vec3& axis, const float amount)
  {
    CPU_FUNC_RANGE();

    FramebufferPtr frmBackup = m_framebuffer;

    m_oneColorAttachmentFramebuffer->Init({0, 0, false, false});

    if (m_averageBlurMaterial == nullptr)
    {
      ShaderPtr vert        = GetShaderManager()->Create<Shader>(ShaderPath("avgBlurVert.shader", true));
      ShaderPtr frag        = GetShaderManager()->Create<Shader>(ShaderPath("avgBlurFrag.shader", true));

      m_averageBlurMaterial = MakeNewPtr<Material>();
      m_averageBlurMaterial->m_vertexShader   = vert;
      m_averageBlurMaterial->m_fragmentShader = frag;
      m_averageBlurMaterial->m_diffuseTexture = nullptr;
    }

    m_averageBlurMaterial->UnInit();
    m_averageBlurMaterial->m_diffuseTexture = source;
    m_averageBlurMaterial->Init();

    m_averageBlurMaterial->UpdateUniformOfThisMaterialsProgram("BlurScale", axis * amount);

    m_oneColorAttachmentFramebuffer->SetColorAttachment(Framebuffer::Attachment::ColorAttachment0, dest);

    SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
    DrawFullQuad(m_averageBlurMaterial);

    SetFramebuffer(frmBackup, GraphicBitFields::None);
  }

  void Renderer::GenerateBRDFLutTexture()
  {
    if (!GetTextureManager()->Exist(TKBrdfLutTexture))
    {
      MaterialPtr prevOverrideMaterial = m_overrideMat;
      FramebufferPtr prevFrameBuffer   = GetFrameBuffer();

      m_overrideMat                    = nullptr;

      TextureSettings set;
      set.InternalFormat = GraphicTypes::FormatRG16F;
      set.Format         = GraphicTypes::FormatRG;
      set.Type           = GraphicTypes::TypeFloat;
      set.GenerateMipMap = false;

      RenderTargetPtr brdfLut =
          MakeNewPtr<RenderTarget>(RHIConstants::BrdfLutTextureSize, RHIConstants::BrdfLutTextureSize, set);
      brdfLut->Init();

      FramebufferPtr utilFramebuffer = MakeNewPtr<Framebuffer>();
      utilFramebuffer->Init({RHIConstants::BrdfLutTextureSize, RHIConstants::BrdfLutTextureSize, false, false});
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
      RenderJobProcessor::CreateRenderJobs({quad}, jobs);
      RenderWithProgramFromMaterial(jobs);

      brdfLut->SetFile(TKBrdfLutTexture);
      GetTextureManager()->Manage(brdfLut);
      m_brdfLut     = brdfLut;

      m_overrideMat = prevOverrideMaterial;
      SetFramebuffer(prevFrameBuffer, GraphicBitFields::None);
    }
  }

  void Renderer::BindProgram(const GpuProgramPtr& program)
  {
    if (m_currentProgram == nullptr || m_currentProgram->m_handle != program->m_handle)
    {
      m_currentProgram = program;
      glUseProgram(program->m_handle);
    }
  }

  void Renderer::FeedUniforms(const GpuProgramPtr& program, const RenderJob& job)
  {
    CPU_FUNC_RANGE();

    // Update camera related uniforms.
    if (m_gpuProgramHasCameraUpdates.find(program->m_handle) == m_gpuProgramHasCameraUpdates.end())
    {
      int uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::VIEW));
      if (uniformLoc != -1)
      {
        glUniformMatrix4fv(uniformLoc, 1, false, &m_view[0][0]);
      }

      uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::PROJECT_VIEW_NO_TR));
      if (uniformLoc != -1)
      {
        glUniformMatrix4fv(uniformLoc, 1, false, &m_projectViewNoTranslate[0][0]);
      }

      uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::CAM_DATA_POS));
      if (uniformLoc != -1)
      {
        glUniform3fv(uniformLoc, 1, &m_camPos.x);
      }

      uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::CAM_DATA_DIR));
      if (uniformLoc != -1)
      {
        glUniform3fv(uniformLoc, 1, &m_camDirection.x);
      }

      uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::CAM_DATA_FAR));
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, m_camFar);
      }

      uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::SHADOW_DISTANCE));
      if (uniformLoc != -1)
      {
        EngineSettings& set = GetEngineSettings();
        glUniform1f(uniformLoc, set.Graphics.ShadowDistance);
      }

      m_gpuProgramHasCameraUpdates.insert(program->m_handle);
    }

    // Update Per frame changing uniforms. ExecuteRenderTasks clears programs at the beginning of the call.
    if (m_gpuProgramHasFrameUpdates.find(program->m_handle) == m_gpuProgramHasFrameUpdates.end())
    {
      int uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::FRAME_COUNT));
      if (uniformLoc != -1)
      {
        glUniform1ui(uniformLoc, m_frameCount);
      }

      uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::ELAPSED_TIME));
      if (uniformLoc != -1)
      {
        glUniform1f(uniformLoc, Main::GetInstance()->TimeSinceStartup() / 1000.0f);
      }

      m_gpuProgramHasFrameUpdates.insert(program->m_handle);
    }

    int uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::COLOR));
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

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::IBL_INTENSITY));
    if (uniformLoc != -1)
    {
      glUniform1f(uniformLoc, m_renderState.iblIntensity);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::IBL_MAX_REFLECTION_LOD));
    if (uniformLoc != -1)
    {
      glUniform1i(uniformLoc, RHIConstants::SpecularIBLLods - 1);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::COLOR_ALPHA));
    if (uniformLoc != -1)
    {
      glUniform1f(uniformLoc, m_mat->GetAlpha());
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::USE_ALPHA_MASK));
    if (uniformLoc != -1)
    {
      glUniform1i(uniformLoc, m_renderState.blendFunction == BlendFunction::ALPHA_MASK);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::ALPHA_MASK_TRESHOLD));
    if (uniformLoc != -1)
    {
      glUniform1f(uniformLoc, m_renderState.alphaMaskTreshold);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::DIFFUSE_TEXTURE_IN_USE));
    if (uniformLoc != -1)
    {
      int diffInUse = (int) (m_mat->m_diffuseTexture != nullptr);
      if (m_renderOnlyLighting)
      {
        diffInUse = false;
      }
      glUniform1i(uniformLoc, diffInUse);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::EMISSIVE_TEXTURE_IN_USE));
    if (uniformLoc != -1)
    {
      int emmInUse = (int) (m_mat->m_emissiveTexture != nullptr);
      glUniform1i(uniformLoc, emmInUse);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::EMISSIVE_COLOR));
    if (uniformLoc != -1)
    {
      glUniform3fv(uniformLoc, 1, &m_mat->m_emissiveColor.x);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::NORMAL_MAP_IN_USE));
    if (uniformLoc != -1)
    {
      int normInUse = (int) (m_mat->m_normalMap != nullptr);
      glUniform1i(uniformLoc, normInUse);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE));
    if (uniformLoc != -1)
    {
      int metRghInUse = (int) (m_mat->m_metallicRoughnessTexture != nullptr);
      glUniform1i(uniformLoc, metRghInUse);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::METALLIC));
    if (uniformLoc != -1)
    {
      glUniform1f(uniformLoc, (GLfloat) m_mat->m_metallic);
    }

    uniformLoc = program->GetUniformLocation(GetUniformName(Uniform::ROUGHNESS));
    if (uniformLoc != -1)
    {
      glUniform1f(uniformLoc, (GLfloat) m_mat->m_roughness);
    }

    for (auto& uniform : program->m_uniformLocationsNEW)
    {
      // Built-in variables.

      GLint loc = uniform.second;

      if (uniform.first == GetUniformName(Uniform::PROJECT_VIEW_MODEL))
      {
        Mat4 mul = m_projectView * m_model;
        glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
      }
      else if (uniform.first == GetUniformName(Uniform::MODEL))
      {
        glUniformMatrix4fv(loc, 1, false, &m_model[0][0]);
      }
      else if (uniform.first == GetUniformName(Uniform::MODEL_NO_TR))
      {
        Mat4 modelNoTr  = m_model;
        modelNoTr[0][3] = 0.0f;
        modelNoTr[1][3] = 0.0f;
        modelNoTr[2][3] = 0.0f;
        modelNoTr[3][3] = 1.0f;
        modelNoTr[3][0] = 0.0f;
        modelNoTr[3][1] = 0.0f;
        modelNoTr[3][2] = 0.0f;
        glUniformMatrix4fv(loc, 1, false, &modelNoTr[0][0]);
      }
      else if (uniform.first == GetUniformName(Uniform::INV_TR_MODEL))
      {
        Mat4 invTrModel = glm::transpose(glm::inverse(m_model));
        glUniformMatrix4fv(loc, 1, false, &invTrModel[0][0]);
      }
      else if (uniform.first == GetUniformName(Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE))
      {
        if (m_mat->m_metallicRoughnessTexture != nullptr)
        {
          if (m_mat->m_metallicRoughnessTexture)
          {
            SetTexture(4, m_mat->m_metallicRoughnessTexture->m_textureId);
          }
        }
      }
      else if (uniform.first == GetUniformName(Uniform::NORMAL_MAP_IN_USE))
      {
        if (m_mat->m_normalMap != nullptr)
        {
          SetTexture(9, m_mat->m_normalMap->m_textureId);
        }
      }
      else if (uniform.first == GetUniformName(Uniform::USE_IBL))
      {
        if (m_renderState.IBLInUse)
        {
          SetTexture(7, m_renderState.irradianceMap);
          SetTexture(15, m_renderState.preFilteredSpecularMap);
          SetTexture(16, m_renderState.brdfLut);
        }
        glUniform1i(loc, (GLint) m_renderState.IBLInUse);
      }
      else if (uniform.first == GetUniformName(Uniform::IBL_ROTATION))
      {
        if (m_renderState.IBLInUse)
        {
          glUniformMatrix4fv(loc, 1, false, &m_iblRotation[0][0]);
        }
      }
      else if (uniform.first == GetUniformName(Uniform::EMISSIVE_TEXTURE_IN_USE))
      {
        if (m_mat->m_emissiveTexture != nullptr)
        {
          SetTexture(1, m_mat->m_emissiveTexture->m_textureId);
        }
      }
      else if (uniform.first == GetUniformName(Uniform::KEY_FRAME_1))
      {
        glUniform1f(loc, job.animData.firstKeyFrame);
      }
      else if (uniform.first == GetUniformName(Uniform::KEY_FRAME_2))
      {
        glUniform1f(loc, job.animData.secondKeyFrame);
      }
      else if (uniform.first == GetUniformName(Uniform::KEY_FRAME_INT_TIME))
      {
        glUniform1f(loc, job.animData.keyFrameInterpolationTime);
      }
      else if (uniform.first == GetUniformName(Uniform::KEY_FRAME_COUNT))
      {
        glUniform1f(loc, job.animData.keyFrameCount);
      }
      else if (uniform.first == GetUniformName(Uniform::IS_ANIMATED))
      {
        glUniform1ui(loc, job.animData.currentAnimation != nullptr);
      }
      else if (uniform.first == GetUniformName(Uniform::BLEND_ANIMATION))
      {
        glUniform1i(loc, job.animData.blendAnimation != nullptr);
      }
      else if (uniform.first == GetUniformName(Uniform::BLEND_FACTOR))
      {
        glUniform1f(loc, job.animData.animationBlendFactor);
      }
      else if (uniform.first == GetUniformName(Uniform::BLEND_KEY_FRAME_1))
      {
        glUniform1f(loc, job.animData.blendFirstKeyFrame);
      }
      else if (uniform.first == GetUniformName(Uniform::BLEND_KEY_FRAME_2))
      {
        glUniform1f(loc, job.animData.blendSecondKeyFrame);
      }
      else if (uniform.first == GetUniformName(Uniform::BLEND_KEY_FRAME_INT_TIME))
      {
        glUniform1f(loc, job.animData.blendKeyFrameInterpolationTime);
      }
      else if (uniform.first == GetUniformName(Uniform::BLEND_KEY_FRAME_COUNT))
      {
        glUniform1f(loc, job.animData.blendKeyFrameCount);
      }
    }

    for (auto& uniform : program->m_uniformsNEW)
    {
      GLint loc = program->GetUniformLocation(uniform.first);

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
    CPU_FUNC_RANGE();

    for (int i = 0; i < (int) job.activeLightCount; i++)
    {
      LightPtr currLight = m_lights[job.lights[i]];

      // Point light uniforms
      if (currLight->GetLightType() == Light::Point)
      {
        PointLight* pLight = static_cast<PointLight*>(currLight.get());

        Vec3 color         = pLight->GetColorVal();
        float intensity    = pLight->GetIntensityVal();
        Vec3 pos           = pLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        float radius       = pLight->GetRadiusVal();

        GLint loc          = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_TYPE), i);
        glUniform1i(loc, (int) 2);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_COLOR), i);
        glUniform3fv(loc, 1, &color.x);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_INTENSITY), i);
        glUniform1f(loc, intensity);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_POS), i);
        glUniform3fv(loc, 1, &pos.x);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_RADIUS), i);
        glUniform1f(loc, radius);
      }
      // Directional light uniforms
      else if (currLight->GetLightType() == Light::Directional)
      {
        DirectionalLight* dLight = static_cast<DirectionalLight*>(currLight.get());
        Vec3 color               = dLight->GetColorVal();
        float intensity          = dLight->GetIntensityVal();
        Vec3 dir                 = dLight->GetComponentFast<DirectionComponent>()->GetDirection();

        GLint loc                = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_TYPE), i);
        glUniform1i(loc, (GLint) 1);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_COLOR), i);
        glUniform3fv(loc, 1, &color.x);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_INTENSITY), i);
        glUniform1f(loc, intensity);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_DIR), i);
        glUniform3fv(loc, 1, &dir.x);
      }
      // Spot light uniforms
      else if (currLight->GetLightType() == Light::Spot)
      {
        SpotLight* sLight = static_cast<SpotLight*>(currLight.get());
        Vec3 color        = sLight->GetColorVal();
        float intensity   = sLight->GetIntensityVal();
        Vec3 pos          = sLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        Vec3 dir          = sLight->GetComponentFast<DirectionComponent>()->GetDirection();
        float radius      = sLight->GetRadiusVal();
        float outAngle    = glm::cos(glm::radians(sLight->GetOuterAngleVal() / 2.0f));
        float innAngle    = glm::cos(glm::radians(sLight->GetInnerAngleVal() / 2.0f));

        GLint loc         = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_TYPE), i);
        glUniform1i(loc, (GLint) 3);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_COLOR), i);
        glUniform3fv(loc, 1, &color.x);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_INTENSITY), i);
        glUniform1f(loc, intensity);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_POS), i);
        glUniform3fv(loc, 1, &pos.x);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_DIR), i);
        glUniform3fv(loc, 1, &dir.x);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_RADIUS), i);
        glUniform1f(loc, radius);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_OUTANGLE), i);
        glUniform1f(loc, outAngle);
        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_INNANGLE), i);
        glUniform1f(loc, innAngle);
      }

      bool castShadow = currLight->GetCastShadowVal();
      if (castShadow)
      {
        GLint loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_PROJVIEWMATRIX), i);
        glUniformMatrix4fv(loc, 1, GL_FALSE, &(currLight->m_shadowMapCameraProjectionViewMatrix)[0][0]);

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_SHADOWMAPCAMFAR), i);
        glUniform1f(loc, currLight->m_shadowMapCameraFar);

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_BLEEDREDUCTION), i);
        glUniform1f(loc, currLight->GetBleedingReductionVal());

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_PCFSAMPLES), i);
        glUniform1i(loc, currLight->GetPCFSamplesVal());

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_PCFRADIUS), i);
        glUniform1f(loc, currLight->GetPCFRadiusVal());

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_SOFTSHADOWS), i);
        glUniform1i(loc, (int) (currLight->GetPCFSamplesVal() > 1));

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_SHADOWATLASLAYER), i);
        glUniform1f(loc, (GLfloat) currLight->m_shadowAtlasLayer);

        const Vec2 coord = currLight->m_shadowAtlasCoord / (float) Renderer::RHIConstants::ShadowAtlasTextureSize;
        loc              = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_SHADOWATLASCOORD), i);
        glUniform2fv(loc, 1, &coord.x);

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_SHADOWATLASRESRATIO), i);
        glUniform1f(loc, currLight->GetShadowResVal() / Renderer::RHIConstants::ShadowAtlasTextureSize);

        loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_SHADOWBIAS), i);
        glUniform1f(loc, currLight->GetShadowBiasVal() * Renderer::RHIConstants::ShadowBiasMultiplier);
      }

      GLuint loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_CASTSHADOW), i);
      glUniform1i(loc, (int) castShadow);
    }

    GLint loc = program->GetUniformLocation(GetUniformName(Uniform::LIGHT_DATA_ACTIVECOUNT));
    glUniform1i(loc, (int) job.activeLightCount);

    // Bind shadow map if activated
    if (m_shadowAtlas != nullptr)
    {
      SetTexture(8, m_shadowAtlas->m_textureId);
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
    CPU_FUNC_RANGE();

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

    mat->UpdateUniformOfThisMaterialsProgram("Exposure", exposure);

    m_oneColorAttachmentFramebuffer->Init({0, 0, false, false});

    // Views for 6 different angles
    CameraPtr cam = MakeNewPtr<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                    glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    for (int i = 0; i < 6; ++i)
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
      if (i > 0)
      {
        AddHWRenderPass();
      }

      SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
      DrawCube(cam, mat);
    }

    CubeMapPtr cubeMap = MakeNewPtr<CubeMap>();
    cubeMap->Consume(cubeMapRt);

    return cubeMap;
  }

  CubeMapPtr Renderer::GenerateDiffuseEnvMap(CubeMapPtr cubemap, uint size)
  {
    CPU_FUNC_RANGE();

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

    m_oneColorAttachmentFramebuffer->Init({0, 0, false, false});

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
      if (i > 0)
      {
        AddHWRenderPass();
      }

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
    CPU_FUNC_RANGE();

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

    m_oneColorAttachmentFramebuffer->Init({0, 0, false, false});

    UVec2 lastViewportSize = m_viewportSize;

    assert(size >= 128 && "Due to RHIConstants::SpecularIBLLods, it can't be lower than this resolution.");
    for (int mip = 0; mip < mipMaps; ++mip)
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

        if (mip != 0 && i != 0)
        {
          AddHWRenderPass();
        }

        mat->UpdateUniformOfThisMaterialsProgram("roughness", (float) mip / (float) mipMaps);
        mat->UpdateUniformOfThisMaterialsProgram("resPerFace", (float) mipSize);

        SetFramebuffer(m_oneColorAttachmentFramebuffer, GraphicBitFields::None);
        SetViewportSize(mipSize, mipSize);

        RHI::SetTexture(GL_TEXTURE_CUBE_MAP, cubemap->m_textureId, 0);

        DrawCube(cam, mat);

        // Copy color attachment to cubemap's correct mip level and face.
        RHI::SetTexture(GL_TEXTURE_CUBE_MAP, cubemapRt->m_textureId, 0);
        glCopyTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, mip, 0, 0, 0, 0, mipSize, mipSize);

        // Set the read cubemap back. When renderer hijacked like this, we need to restore its state manually.
        RHI::SetTexture(GL_TEXTURE_CUBE_MAP, cubemap->m_textureId, 0);
      }
    }

    SetFramebuffer(nullptr, GraphicBitFields::None);

    CubeMapPtr newCubeMap = MakeNewPtr<CubeMap>();
    newCubeMap->Consume(cubemapRt);

    return newCubeMap;
  }

} // namespace ToolKit
