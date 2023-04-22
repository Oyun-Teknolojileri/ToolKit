#include "Renderer.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Drawable.h"
#include "EnvironmentComponent.h"
#include "GradientSky.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Pass.h"
#include "ResourceComponent.h"
#include "Scene.h"
#include "Shader.h"
#include "ShaderReflectionCache.h"
#include "Skeleton.h"
#include "Surface.h"
#include "Texture.h"
#include "ToolKit.h"
#include "UIManager.h"
#include "Viewport.h"
#include "gles2.h"

#include <algorithm>
#include <random>

#include "DebugNew.h"

namespace ToolKit
{

#define TK_LUT_TEXTURE "GLOBAL_BRDF_LUT_TEXTURE"

  Renderer::Renderer()
  {
    m_uiCamera        = new Camera();
    m_utilFramebuffer = std::make_shared<Framebuffer>();
  }

  Renderer::~Renderer()
  {
    SafeDel(m_uiCamera);
    m_utilFramebuffer      = nullptr;
    m_gaussianBlurMaterial = nullptr;
    m_averageBlurMaterial  = nullptr;
    m_copyFb               = nullptr;
    m_copyMaterial         = nullptr;

    m_mat                  = nullptr;
    m_aoMat                = nullptr;
    m_framebuffer          = nullptr;
    m_shadowAtlas          = nullptr;

    m_programs.clear();
  }

  int Renderer::GetMaxArrayTextureLayers()
  {
    if (m_maxArrayTextureLayers == -1)
    {
      glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &m_maxArrayTextureLayers);
    }
    return m_maxArrayTextureLayers;
  }

  void Renderer::SetCameraLens(Camera* cam)
  {
    float aspect = (float) m_viewportSize.x / (float) m_viewportSize.y;
    if (!cam->IsOrtographic())
    {
      cam->SetLens(cam->Fov(), aspect, cam->Near(), cam->Far());
    }
    else
    {
      float width  = m_viewportSize.x * 0.5f;
      float height = m_viewportSize.y * 0.5f;
      cam->SetLens(-width, width, -height, height, cam->Near(), cam->Far());
    }
  }

  /**
   * DEPRECATED
   * Renders given UILayer to given Viewport.
   * @param layer UILayer that will be rendered.
   * @param viewport that UILayer will be rendered with.
   */
  void Renderer::RenderUI(Viewport* viewport, UILayer* layer)
  {
    float halfWidth  = viewport->m_wndContentAreaSize.x * 0.5f;
    float halfHeight = viewport->m_wndContentAreaSize.y * 0.5f;

    m_uiCamera->SetLens(-halfWidth,
                        halfWidth,
                        -halfHeight,
                        halfHeight,
                        0.5f,
                        1000.0f);

    EntityRawPtrArray entities = layer->m_scene->GetEntities();
    // RenderEntities(entities, m_uiCamera, viewport);
  }

  void Renderer::Render(const RenderJob& job,
                        Camera* cam,
                        const LightRawPtrArray& lights)
  {
    // Make ibl assignments.
    m_renderState.IBLInUse = false;
    if (EnvironmentComponentPtr envCom = job.EnvironmentVolume)
    {
      m_renderState.iblIntensity           = envCom->GetIntensityVal();

      HdriPtr hdriPtr                      = envCom->GetHdriVal();
      CubeMapPtr irradianceCubemap         = hdriPtr->m_irradianceCubemap;
      CubeMapPtr preFilteredSpecularIBLMap = hdriPtr->m_prefilteredEnvMap;
      RenderTargetPtr brdfLut =
          GetTextureManager()->Create<RenderTarget>(TK_LUT_TEXTURE);

      if (irradianceCubemap && preFilteredSpecularIBLMap && brdfLut)
      {
        m_renderState.irradianceMap = irradianceCubemap->m_textureId;
        m_renderState.preFilteredSpecularMap =
            preFilteredSpecularIBLMap->m_textureId;
        m_renderState.brdfLut  = brdfLut->m_textureId;

        m_renderState.IBLInUse = true;
        if (Entity* env = envCom->m_entity)
        {
          m_iblRotation =
              Mat4(env->m_node->GetOrientation(TransformationSpace::TS_WORLD));
        }
      }
    }

    // Skeleton Component is used by all meshes of an entity.
    const auto& updateAndBindSkinningTextures = [&job, this]()
    {
      if (!job.Mesh->IsSkinned())
      {
        return;
      }

      SkeletonPtr skel = static_cast<SkinMesh*>(job.Mesh)->m_skeleton;
      if (skel == nullptr)
      {
        return;
      }

      SkeletonComponentPtr skCom = job.SkeletonCmp;
      if (skCom == nullptr)
      {
        return;
      }

      // Bind bone textures
      // This is valid because these slots will be used by every shader program
      // below (Renderer::TextureSlot system). But bone count can't be bound
      // here because its location changes every shader program.

      SetTexture(2, skel->m_bindPoseTexture->m_textureId);
      SetTexture(3, skCom->m_map->boneTransformNodeTexture->m_textureId);
      skCom->m_map->UpdateGPUTexture();
    };

    updateAndBindSkinningTextures();

    SetProjectViewModel(job.WorldTransform, cam);
    m_lights = lights;
    m_cam    = cam;
    job.Mesh->Init();
    job.Material->Init();

    // Set render material.
    m_mat = m_overrideMat != nullptr ? m_overrideMat : job.Material;

    if (m_mat == nullptr)
    {
      assert(false);
      m_mat = GetMaterialManager()->GetCopyOfDefaultMaterial();
    }

    m_mat->Init();
    ProgramPtr prg =
        CreateProgram(m_mat->m_vertexShader, m_mat->m_fragmentShader);
    BindProgram(prg);

    auto activateSkinning = [prg, &job](uint isSkinned)
    {
      GLint isSkinnedLoc = glGetUniformLocation(prg->m_handle, "isSkinned");
      glUniform1ui(isSkinnedLoc, isSkinned);

      if (job.SkeletonCmp == nullptr)
      {
        return;
      }

      if (isSkinned)
      {
        GLint numBonesLoc = glGetUniformLocation(prg->m_handle, "numBones");
        float boneCount =
            (float) job.SkeletonCmp->GetSkeletonResourceVal()->m_bones.size();

        glUniform1fv(numBonesLoc, 1, &boneCount);
      }
    };

    Mesh* mesh = job.Mesh;
    activateSkinning(mesh->IsSkinned());

    RenderState* rs = m_mat->GetRenderState();
    SetRenderState(rs);
    FeedUniforms(prg);

    glBindVertexArray(mesh->m_vaoId);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
    SetVertexLayout(mesh->m_vertexLayout);

    if (mesh->m_indexCount != 0)
    {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_vboIndexId);
      glDrawElements((GLenum) rs->drawType,
                     mesh->m_indexCount,
                     GL_UNSIGNED_INT,
                     nullptr);
    }
    else
    {
      glDrawArrays((GLenum) rs->drawType, 0, mesh->m_vertexCount);
    }
  }

  void Renderer::Render(const RenderJobArray& jobArray,
                        Camera* cam,
                        const LightRawPtrArray& lights)
  {
    for (const RenderJob& rj : jobArray)
    {
      Render(rj, cam, lights);
    }
  }

  void Renderer::Render2d(Surface* object, glm::ivec2 screenDimensions)
  {
    static ShaderPtr vertexShader = GetShaderManager()->Create<Shader>(
        ShaderPath("defaultVertex.shader", true));
    static ShaderPtr fragShader = GetShaderManager()->Create<Shader>(
        ShaderPath("unlitFrag.shader", true));
    static ProgramPtr prog = CreateProgram(vertexShader, fragShader);
    BindProgram(prog);

    MeshPtr mesh = object->GetMeshComponent()->GetMeshVal();
    mesh->Init();

    RenderState* rs = mesh->m_material->GetRenderState();
    SetRenderState(rs);

    GLint pvloc = glGetUniformLocation(prog->m_handle, "ProjectViewModel");
    Mat4 pm     = glm::ortho(0.0f,
                         static_cast<float>(screenDimensions.x),
                         0.0f,
                         static_cast<float>(screenDimensions.y),
                         0.0f,
                         100.0f);

    Mat4 mul = pm * object->m_node->GetTransform(TransformationSpace::TS_WORLD);

    glUniformMatrix4fv(pvloc, 1, false, reinterpret_cast<float*>(&mul));

    glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);

    glDrawArrays((GLenum) rs->drawType, 0, mesh->m_vertexCount);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void Renderer::Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions)
  {
    Surface* surface = object->GetCurrentSurface();

    Node* backup     = surface->m_node;
    surface->m_node  = object->m_node;

    Render2d(surface, screenDimensions);

    surface->m_node = backup;
  }

  void Renderer::SetRenderState(const RenderState* const state)
  {
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

    // TODO: Cihan move SetTexture to render path.
    if (m_mat)
    {
      if (m_mat->m_cubeMap)
      {
        SetTexture(6, m_mat->m_cubeMap->m_textureId);
      }

      if (m_mat->m_diffuseTexture)
      {
        SetTexture(0, m_mat->m_diffuseTexture->m_textureId);
      }

      if (m_mat->m_emissiveTexture)
      {
        SetTexture(1, m_mat->m_emissiveTexture->m_textureId);
      }

      if (m_mat->m_metallicRoughnessTexture)
      {
        SetTexture(4, m_mat->m_metallicRoughnessTexture->m_textureId);
      }

      if (m_mat->m_normalMap)
      {
        SetTexture(9, m_mat->m_normalMap->m_textureId);
      }

      if (m_mat->GetRenderState()->IBLInUse) {}
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
                                bool clear,
                                const Vec4& color)
  {
    if (fb != m_framebuffer)
    {
      if (fb != nullptr)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, fb->GetFboId());
        FramebufferSettings fbSet = fb->GetSettings();
        SetViewportSize(fbSet.width, fbSet.height);
      }
      else
      {
        // Set backbuffer as draw area.
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        SetViewportSize(m_windowSize.x, m_windowSize.y);
      }
    }

    if (clear)
    {
      ClearBuffer(GraphicBitFields::DepthStencilBits, Vec4(1.0f));
      ClearColorBuffer(color);
    }

    m_framebuffer = fb;
  }

  void Renderer::SetFramebuffer(FramebufferPtr fb, bool clear)
  {
    SetFramebuffer(fb, clear, m_clearColor);
  }

  void Renderer::SwapFramebuffer(FramebufferPtr& fb,
                                 bool clear,
                                 const Vec4& color)
  {
    FramebufferPtr& tmp1 = fb;
    FramebufferPtr tmp2  = m_framebuffer;
    SetFramebuffer(fb, clear, color);
    tmp1.swap(tmp2);
  }

  void Renderer::SwapFramebuffer(FramebufferPtr& fb, bool clear)
  {
    SwapFramebuffer(fb, clear, m_clearColor);
  }

  FramebufferPtr Renderer::GetFrameBuffer() { return m_framebuffer; }

  void Renderer::ClearFrameBuffer(FramebufferPtr fb, const Vec4& color)
  {
    SwapFramebuffer(fb, true, color);
    SwapFramebuffer(fb, false);
  }

  void Renderer::ClearColorBuffer(const Vec4& color)
  {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear((GLbitfield) GraphicBitFields::ColorBits);
  }

  void Renderer::ClearBuffer(GraphicBitFields fields, const Vec4& value)
  {
    glClearColor(value.r, value.g, value.b, value.a);
    glClear((GLbitfield) fields);
  }

  void Renderer::ColorMask(bool r, bool g, bool b, bool a)
  {
    glColorMask(r, g, b, a);
  }

  void Renderer::CopyFrameBuffer(FramebufferPtr src,
                                 FramebufferPtr dest,
                                 GraphicBitFields fields)
  {
    GLuint srcId = 0;
    uint width   = m_windowSize.x;
    uint height  = m_windowSize.y;

    if (src)
    {
      FramebufferSettings fbs = src->GetSettings();
      width                   = fbs.width;
      height                  = fbs.height;
      srcId                   = src->GetFboId();
    }

    dest->ReconstructIfNeeded(width, height);

    GLint drawFboId = 0, readFboId = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest->GetFboId());
    glBlitFramebuffer(0,
                      0,
                      width,
                      height,
                      0,
                      0,
                      width,
                      height,
                      (GLbitfield) fields,
                      GL_NEAREST);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFboId);
  }

  void Renderer::SetViewport(Viewport* viewport)
  {
    SetFramebuffer(viewport->m_framebuffer);
  }

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
    static ShaderPtr fullQuadVert = GetShaderManager()->Create<Shader>(
        ShaderPath("fullQuadVert.shader", true));
    static MaterialPtr material = std::make_shared<Material>();
    material->UnInit();

    material->m_vertexShader   = fullQuadVert;
    material->m_fragmentShader = fragmentShader;
    material->Init();

    DrawFullQuad(material);
  }

  void Renderer::DrawFullQuad(MaterialPtr mat)
  {
    static Camera quadCam;
    static Quad quad;
    quad.GetMeshComponent()->GetMeshVal()->m_material = mat;

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJob(&quad, jobs);
    Render(jobs, &quadCam);
  }

  void Renderer::DrawCube(Camera* cam, MaterialPtr mat, const Mat4& transform)
  {
    Cube cube;
    cube.Generate(cube.GetMeshComponent(), cube.GetCubeScaleVal());
    cube.m_node->SetTransform(transform);

    MaterialComponentPtr matc = cube.GetMaterialComponent();
    if (matc == nullptr)
    {
      cube.AddComponent(new MaterialComponent);
    }
    cube.GetMaterialComponent()->SetFirstMaterial(mat);

    RenderJobArray jobs;
    RenderJobProcessor::CreateRenderJob(&cube, jobs);
    Render(jobs, cam);
  }

  void Renderer::CopyTexture(TexturePtr source, TexturePtr dest)
  {
    assert(source->m_width == dest->m_width &&
           source->m_height == dest->m_height &&
           "Sizes of the textures are not the same.");

    assert(source->m_initiated && dest->m_initiated &&
           "Texture is not initialized.");

    assert(source);

    if (m_copyFb == nullptr)
    {
      m_copyFb = std::make_shared<Framebuffer>();
      m_copyFb->Init(
          {(uint) source->m_width, (uint) source->m_height, false, false});
    }

    RenderTargetPtr rt = std::static_pointer_cast<RenderTarget>(dest);
    m_copyFb->SetAttachment(Framebuffer::Attachment::ColorAttachment0, rt);

    // Set and clear fb
    FramebufferPtr lastFb = m_framebuffer;
    SetFramebuffer(m_copyFb, true, Vec4(0.0f));

    // Render to texture
    if (m_copyMaterial == nullptr)
    {
      m_copyMaterial                 = std::make_shared<Material>();
      m_copyMaterial->m_vertexShader = GetShaderManager()->Create<Shader>(
          ShaderPath("copyTextureVert.shader", true));
      m_copyMaterial->m_fragmentShader = GetShaderManager()->Create<Shader>(
          ShaderPath("copyTextureFrag.shader", true));
    }

    m_copyMaterial->UnInit();
    m_copyMaterial->m_diffuseTexture = source;
    m_copyMaterial->Init();

    DrawFullQuad(m_copyMaterial);
    SetFramebuffer(lastFb, false);
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
    FramebufferPtr frmBackup = m_framebuffer;

    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({0, 0, false, false});

    if (m_gaussianBlurMaterial == nullptr)
    {
      ShaderPtr vert = GetShaderManager()->Create<Shader>(
          ShaderPath("gausBlur7x1Vert.shader", true));

      ShaderPtr frag = GetShaderManager()->Create<Shader>(
          ShaderPath("gausBlur7x1Frag.shader", true));

      m_gaussianBlurMaterial                   = std::make_shared<Material>();
      m_gaussianBlurMaterial->m_vertexShader   = vert;
      m_gaussianBlurMaterial->m_fragmentShader = frag;
      m_gaussianBlurMaterial->m_diffuseTexture = nullptr;
    }

    m_gaussianBlurMaterial->UnInit();
    m_gaussianBlurMaterial->m_diffuseTexture = source;
    m_gaussianBlurMaterial->m_fragmentShader->SetShaderParameter(
        "BlurScale",
        ParameterVariant(axis * amount));
    m_gaussianBlurMaterial->Init();

    m_utilFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                     dest);

    SetFramebuffer(m_utilFramebuffer, true, Vec4(1.0f));
    DrawFullQuad(m_gaussianBlurMaterial);

    SetFramebuffer(frmBackup, false);
  }

  void Renderer::ApplyAverageBlur(const TexturePtr source,
                                  RenderTargetPtr dest,
                                  const Vec3& axis,
                                  const float amount)
  {
    FramebufferPtr frmBackup = m_framebuffer;

    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({0, 0, false, false});

    if (m_averageBlurMaterial == nullptr)
    {
      ShaderPtr vert = GetShaderManager()->Create<Shader>(
          ShaderPath("avgBlurVert.shader", true));

      ShaderPtr frag = GetShaderManager()->Create<Shader>(
          ShaderPath("avgBlurFrag.shader", true));

      m_averageBlurMaterial                   = std::make_shared<Material>();
      m_averageBlurMaterial->m_vertexShader   = vert;
      m_averageBlurMaterial->m_fragmentShader = frag;
      m_averageBlurMaterial->m_diffuseTexture = nullptr;
    }

    m_averageBlurMaterial->UnInit();
    m_averageBlurMaterial->m_diffuseTexture = source;
    m_averageBlurMaterial->m_fragmentShader->SetShaderParameter(
        "BlurScale",
        ParameterVariant(axis * amount));

    m_averageBlurMaterial->Init();

    m_utilFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                     dest);

    SetFramebuffer(m_utilFramebuffer, true, Vec4(1.0f));
    DrawFullQuad(m_averageBlurMaterial);

    SetFramebuffer(frmBackup, false);
  }

  void Renderer::SetProjectViewModel(const Mat4& model, Camera* cam)
  {
    m_view    = cam->GetViewMatrix();
    m_project = cam->GetProjectionMatrix();
    m_model   = model;
  }

  void Renderer::BindProgram(ProgramPtr program)
  {
    if (m_currentProgram == program->m_handle)
    {
      return;
    }

    m_currentProgram = program->m_handle;
    glUseProgram(program->m_handle);
  }

  void Renderer::LinkProgram(GLuint program, GLuint vertexP, GLuint fragmentP)
  {
    glAttachShader(program, vertexP);
    glAttachShader(program, fragmentP);

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
      GLint infoLen = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen > 1)
      {
        char* log = new char[infoLen];
        glGetProgramInfoLog(program, infoLen, nullptr, log);
        GetLogger()->Log(log);

        assert(linked);
        SafeDelArray(log);
      }

      glDeleteProgram(program);
    }
  }

  ProgramPtr Renderer::CreateProgram(ShaderPtr vertex, ShaderPtr fragment)
  {
    assert(vertex);
    assert(fragment);
    vertex->Init();
    fragment->Init();

    String tag;
    tag = vertex->m_tag + fragment->m_tag;
    if (m_programs.find(tag) == m_programs.end())
    {
      ProgramPtr program = std::make_shared<Program>(vertex, fragment);
      program->m_handle  = glCreateProgram();
      LinkProgram(program->m_handle,
                  vertex->m_shaderHandle,
                  fragment->m_shaderHandle);
      glUseProgram(program->m_handle);
      for (ubyte slotIndx = 0; slotIndx < m_rhiSettings::textureSlotCount;
           slotIndx++)
      {
        GLint loc = glGetUniformLocation(
            program->m_handle,
            ("s_texture" + std::to_string(slotIndx)).c_str());
        if (loc != -1)
        {
          glUniform1i(loc, slotIndx);
        }
      }

      m_programs[program->m_tag] = program;
    }

    return m_programs[tag];
  }

  void Renderer::FeedUniforms(ProgramPtr program)
  {
    for (ShaderPtr shader : program->m_shaders)
    {
      shader->UpdateShaderParameters();

      // Built-in variables.
      for (Uniform uni : shader->m_uniforms)
      {
        switch (uni)
        {
        case Uniform::PROJECT_MODEL_VIEW:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::PROJECT_MODEL_VIEW));
          Mat4 mul = m_project * m_view * m_model;
          glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
        }
        break;
        case Uniform::VIEW:
        {
          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::VIEW));
          glUniformMatrix4fv(loc, 1, false, &m_view[0][0]);
        }
        case Uniform::MODEL:
        {
          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::MODEL));
          glUniformMatrix4fv(loc, 1, false, &m_model[0][0]);
        }
        break;
        case Uniform::INV_TR_MODEL:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::INV_TR_MODEL));
          Mat4 invTrModel = glm::transpose(glm::inverse(m_model));
          glUniformMatrix4fv(loc, 1, false, &invTrModel[0][0]);
        }
        break;
        case Uniform::LIGHT_DATA:
        {
          FeedLightUniforms(program);
        }
        break;
        case Uniform::CAM_DATA:
        {
          if (m_cam == nullptr)
            break;

          Camera::CamData data     = m_cam->GetData();
          String uniformStructName = GetUniformName(Uniform::CAM_DATA);
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   (uniformStructName + ".pos").c_str());
          glUniform3fv(loc, 1, &data.pos.x);
          loc = glGetUniformLocation(program->m_handle,
                                     (uniformStructName + ".dir").c_str());
          glUniform3fv(loc, 1, &data.dir.x);
          loc = glGetUniformLocation(program->m_handle,
                                     (uniformStructName + ".far").c_str());
          glUniform1f(loc, data.far);
        }
        break;
        case Uniform::COLOR:
        {
          if (m_mat == nullptr)
            break;

          Vec4 color = Vec4(m_mat->m_color, m_mat->m_alpha);
          if (m_mat->GetRenderState()->blendFunction == BlendFunction::NONE)
          {
            color.a = 1.0f;
          }

          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::COLOR));
          if (m_renderOnlyLighting)
          {
            color = Vec4(1.0f, 1.0f, 1.0f, color.a);
          }
          glUniform4fv(loc, 1, &color.x);
        }
        break;
        case Uniform::FRAME_COUNT:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::FRAME_COUNT));
          glUniform1ui(loc, m_frameCount);
        }
        break;
        case Uniform::EXPOSURE:
        {
          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::EXPOSURE));
          glUniform1f(loc, shader->m_shaderParams["Exposure"].GetVar<float>());
        }
        break;
        case Uniform::PROJECT_VIEW_NO_TR:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::PROJECT_VIEW_NO_TR));

          // Zero translate variables in model matrix
          Mat4 view  = m_view;
          view[0][3] = 0.0f;
          view[1][3] = 0.0f;
          view[2][3] = 0.0f;
          view[3][3] = 1.0f;
          view[3][0] = 0.0f;
          view[3][1] = 0.0f;
          view[3][2] = 0.0f;

          Mat4 mul   = m_project * view;
          glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
        }
        break;
        case Uniform::USE_IBL:
        {
          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::USE_IBL));
          glUniform1i(loc, (GLint) m_renderState.IBLInUse);
        }
        break;
        case Uniform::IBL_INTENSITY:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::IBL_INTENSITY));
          glUniform1f(loc, m_renderState.iblIntensity);
        }
        break;
        case Uniform::IBL_IRRADIANCE:
        {
          SetTexture(7, m_renderState.irradianceMap);
          SetTexture(15, m_renderState.preFilteredSpecularMap);
          SetTexture(16, m_renderState.brdfLut);
        }
        break;
        case Uniform::DIFFUSE_TEXTURE_IN_USE:
        {
          GLint loc = glGetUniformLocation(
              program->m_handle,
              GetUniformName(Uniform::DIFFUSE_TEXTURE_IN_USE));
          int v = (int) (m_mat->m_diffuseTexture != nullptr);
          if (m_renderOnlyLighting)
          {
            v = false;
          }
          glUniform1i(loc, v);
        }
        break;
        case Uniform::COLOR_ALPHA:
        {
          if (m_mat == nullptr)
            break;

          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::COLOR_ALPHA));
          glUniform1f(loc, m_mat->m_alpha);
        }
        break;
        case Uniform::IBL_ROTATION:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::IBL_ROTATION));

          glUniformMatrix4fv(loc, 1, false, &m_iblRotation[0][0]);
        }
        break;
        case Uniform::USE_ALPHA_MASK:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::USE_ALPHA_MASK));
          glUniform1i(loc,
                      m_renderState.blendFunction == BlendFunction::ALPHA_MASK);
        }
        break;
        case Uniform::ALPHA_MASK_TRESHOLD:
        {
          GLint loc = glGetUniformLocation(
              program->m_handle,
              GetUniformName(Uniform::ALPHA_MASK_TRESHOLD));
          glUniform1f(loc, m_renderState.alphaMaskTreshold);
        }
        break;
        case Uniform::EMISSIVE_COLOR:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::EMISSIVE_COLOR));
          glUniform3fv(loc, 1, &m_mat->m_emissiveColor.x);
        }
        break;
        case Uniform::EMISSIVE_TEXTURE_IN_USE:
        {
          GLint loc = glGetUniformLocation(
              program->m_handle,
              GetUniformName(Uniform::EMISSIVE_TEXTURE_IN_USE));
          int v = (int) (m_mat->m_emissiveTexture != nullptr);
          glUniform1i(loc, v);
        }
        break;
        case Uniform::UNUSEDSLOT_3:
          assert(false);
          break;
        case Uniform::METALLIC:
        {
          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::METALLIC));
          glUniform1f(loc, (GLfloat) m_mat->m_metallic);
        }
        break;
        case Uniform::ROUGHNESS:
        {
          GLint loc = glGetUniformLocation(program->m_handle,
                                           GetUniformName(Uniform::ROUGHNESS));
          glUniform1f(loc, (GLfloat) m_mat->m_roughness);
        }
        break;
        case Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE:
        {
          GLint loc = glGetUniformLocation(
              program->m_handle,
              GetUniformName(Uniform::METALLIC_ROUGHNESS_TEXTURE_IN_USE));
          glUniform1i(loc,
                      (int) (m_mat->m_metallicRoughnessTexture != nullptr));
        }
        break;
        case Uniform::NORMAL_MAP_IN_USE:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::NORMAL_MAP_IN_USE));
          glUniform1i(loc, (int) (m_mat->m_normalMap != nullptr));
        }
        break;
        case Uniform::IBL_MAX_REFLECTION_LOD:
        {
          GLint loc = glGetUniformLocation(
              program->m_handle,
              GetUniformName(Uniform::IBL_MAX_REFLECTION_LOD));
          glUniform1i(loc, RHIConstants::specularIBLLods - 1);
        }
        break;
        case Uniform::ELAPSED_TIME:
        {
          GLint loc =
              glGetUniformLocation(program->m_handle,
                                   GetUniformName(Uniform::ELAPSED_TIME));
          glUniform1f(loc, Main::GetInstance()->TimeSinceStartup() / 1000.0f);
        }
        break;
        default:
          assert(false);
          break;
        }
      }

      // Custom variables.
      for (auto& var : shader->m_shaderParams)
      {
        GLint loc = glGetUniformLocation(program->m_handle, var.first.c_str());
        if (loc == -1)
        {
          continue;
        }

        switch (var.second.GetType())
        {
        case ParameterVariant::VariantType::Bool:
          glUniform1ui(loc, var.second.GetVar<bool>());
          break;
        case ParameterVariant::VariantType::Float:
          glUniform1f(loc, var.second.GetVar<float>());
          break;
        case ParameterVariant::VariantType::Int:
          glUniform1i(loc, var.second.GetVar<int>());
          break;
        case ParameterVariant::VariantType::UInt:
          glUniform1ui(loc, var.second.GetVar<uint>());
          break;
        case ParameterVariant::VariantType::Vec2:
          glUniform2fv(loc,
                       1,
                       reinterpret_cast<float*>(&var.second.GetVar<Vec2>()));
          break;
        case ParameterVariant::VariantType::Vec3:
          glUniform3fv(loc,
                       1,
                       reinterpret_cast<float*>(&var.second.GetVar<Vec3>()));
          break;
        case ParameterVariant::VariantType::Vec4:
          glUniform4fv(loc,
                       1,
                       reinterpret_cast<float*>(&var.second.GetVar<Vec4>()));
          break;
        case ParameterVariant::VariantType::Mat3:
          glUniformMatrix3fv(
              loc,
              1,
              false,
              reinterpret_cast<float*>(&var.second.GetVar<Mat3>()));
          break;
        case ParameterVariant::VariantType::Mat4:
          glUniformMatrix4fv(
              loc,
              1,
              false,
              reinterpret_cast<float*>(&var.second.GetVar<Mat4>()));
          break;
        default:
          assert(false && "Invalid type.");
          break;
        }
      }
    }
  }

  void Renderer::FeedLightUniforms(ProgramPtr program)
  {
    size_t lightSize =
        glm::min(m_lights.size(), m_rhiSettings::maxLightsPerObject);
    for (size_t i = 0; i < lightSize; i++)
    {
      Light* currLight = m_lights[i];

      EntityType type  = currLight->GetType();

      // Point light uniforms
      if (type == EntityType::Entity_PointLight)
      {
        Vec3 color      = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 pos =
            currLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        float radius = static_cast<PointLight*>(currLight)->GetRadiusVal();

        GLuint loc   = glGetUniformLocation(program->m_handle,
                                          g_lightTypeStrCache[i].c_str());
        glUniform1i(loc, static_cast<GLuint>(2));
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightColorStrCache[i].c_str());
        glUniform3fv(loc, 1, &color.x);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightIntensityStrCache[i].c_str());
        glUniform1f(loc, intensity);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightPosStrCache[i].c_str());
        glUniform3fv(loc, 1, &pos.x);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightRadiusStrCache[i].c_str());
        glUniform1f(loc, radius);
      }
      // Directional light uniforms
      else if (type == EntityType::Entity_DirectionalLight)
      {
        Vec3 color      = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 dir        = static_cast<DirectionalLight*>(currLight)
                       ->GetComponent<DirectionComponent>()
                       ->GetDirection();

        GLuint loc = glGetUniformLocation(program->m_handle,
                                          g_lightTypeStrCache[i].c_str());
        glUniform1i(loc, static_cast<GLuint>(1));
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightColorStrCache[i].c_str());
        glUniform3fv(loc, 1, &color.x);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightIntensityStrCache[i].c_str());
        glUniform1f(loc, intensity);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightDirStrCache[i].c_str());
        glUniform3fv(loc, 1, &dir.x);
      }
      // Spot light uniforms
      else if (type == EntityType::Entity_SpotLight)
      {
        Vec3 color      = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 pos =
            currLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        SpotLight* spotLight = static_cast<SpotLight*>(currLight);
        Vec3 dir =
            spotLight->GetComponent<DirectionComponent>()->GetDirection();
        float radius = spotLight->GetRadiusVal();
        float outAngle =
            glm::cos(glm::radians(spotLight->GetOuterAngleVal() / 2.0f));
        float innAngle =
            glm::cos(glm::radians(spotLight->GetInnerAngleVal() / 2.0f));

        GLuint loc = glGetUniformLocation(program->m_handle,
                                          g_lightTypeStrCache[i].c_str());
        glUniform1i(loc, static_cast<GLuint>(3));
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightColorStrCache[i].c_str());
        glUniform3fv(loc, 1, &color.x);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightIntensityStrCache[i].c_str());
        glUniform1f(loc, intensity);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightPosStrCache[i].c_str());
        glUniform3fv(loc, 1, &pos.x);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightDirStrCache[i].c_str());
        glUniform3fv(loc, 1, &dir.x);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightRadiusStrCache[i].c_str());
        glUniform1f(loc, radius);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightOuterAngleStrCache[i].c_str());
        glUniform1f(loc, outAngle);
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightInnerAngleStrCache[i].c_str());
        glUniform1f(loc, innAngle);
      }

      bool castShadow = currLight->GetCastShadowVal();
      if (castShadow)
      {
        GLint loc = glGetUniformLocation(
            program->m_handle,
            g_lightprojectionViewMatrixStrCache[i].c_str());
        glUniformMatrix4fv(
            loc,
            1,
            GL_FALSE,
            &(currLight->m_shadowMapCameraProjectionViewMatrix)[0][0]);

        loc =
            glGetUniformLocation(program->m_handle,
                                 g_lightShadowMapCameraFarStrCache[i].c_str());
        glUniform1f(loc, currLight->m_shadowMapCameraFar);

        loc = glGetUniformLocation(program->m_handle,
                                   g_lightBleedingReductionStrCache[i].c_str());
        glUniform1f(loc, currLight->GetLightBleedingReductionVal());

        loc = glGetUniformLocation(program->m_handle,
                                   g_lightPCFSamplesStrCache[i].c_str());
        glUniform1i(loc, currLight->GetPCFSamplesVal());

        loc = glGetUniformLocation(program->m_handle,
                                   g_lightPCFRadiusStrCache[i].c_str());
        glUniform1f(loc, currLight->GetPCFRadiusVal());

        loc = glGetUniformLocation(program->m_handle,
                                   g_lightsoftShadowsStrCache[i].c_str());
        glUniform1i(loc, (int) (currLight->GetPCFSamplesVal() > 1));

        loc = glGetUniformLocation(program->m_handle,
                                   g_lightShadowAtlasLayerStrCache[i].c_str());
        glUniform1f(loc, (GLfloat) currLight->m_shadowAtlasLayer);

        const Vec2 coord =
            currLight->m_shadowAtlasCoord /
            (float) Renderer::m_rhiSettings::g_shadowAtlasTextureSize;
        loc = glGetUniformLocation(program->m_handle,
                                   g_lightShadowAtlasCoordStrCache[i].c_str());
        glUniform2fv(loc, 1, &coord.x);

        loc =
            glGetUniformLocation(program->m_handle,
                                 g_lightShadowAtlasResRatioStrCache[i].c_str());
        glUniform1f(loc,
                    currLight->GetShadowResVal() /
                        Renderer::m_rhiSettings::g_shadowAtlasTextureSize);

        loc = glGetUniformLocation(program->m_handle,
                                   g_lightShadowBiasStrCache[i].c_str());
        glUniform1f(loc,
                    currLight->GetShadowBiasVal() * g_shadowBiasMultiplier);
      }

      GLuint loc = glGetUniformLocation(program->m_handle,
                                        g_lightCastShadowStrCache[i].c_str());
      glUniform1i(loc, static_cast<int>(castShadow));
    }

    GLint loc =
        glGetUniformLocation(program->m_handle, "LightData.activeCount");
    glUniform1i(loc, static_cast<int>(m_lights.size()));

    // Bind shadow map if activated
    if (m_shadowAtlas != nullptr)
    {
      SetTexture(8, m_shadowAtlas->m_textureId);
    }
  }

  void Renderer::SetTexture(ubyte slotIndx, uint textureId)
  {
    assert(slotIndx < 17 && "You exceed texture slot count");
    m_textureSlots[slotIndx] = textureId;
    glActiveTexture(GL_TEXTURE0 + slotIndx);

    static const GLenum textureTypeLut[17] = {
        GL_TEXTURE_2D,       // 0 -> Color Texture
        GL_TEXTURE_2D,       // 1 -> Emissive Texture
        GL_TEXTURE_2D,       // 2 -> Skinning information
        GL_TEXTURE_2D,       // 3 -> Skinning information
        GL_TEXTURE_2D,       // 4 -> Metallic Roughness Texture
        GL_TEXTURE_2D,       // 5 -> AO Texture
        GL_TEXTURE_CUBE_MAP, // 6 -> Cubemap
        GL_TEXTURE_CUBE_MAP, // 7 -> Irradiance Map
        GL_TEXTURE_2D_ARRAY, // 8 -> Shadow Atlas
        GL_TEXTURE_2D,       // 9 -> Normal map, gbuffer position
        GL_TEXTURE_2D,       // 10 -> gBuffer normal texture
        GL_TEXTURE_2D,       // 11 -> gBuffer color texture
        GL_TEXTURE_2D,       // 12 -> gBuffer emissive texture
        GL_TEXTURE_2D,       // 13 -> Light Data Texture
        GL_TEXTURE_2D,       // 14 -> gBuffer metallic roughness texture
        GL_TEXTURE_CUBE_MAP, // 15 -> IBL Specular Pre-Filtered Map
        GL_TEXTURE_2D        // 16 -> IBL BRDF Lut
    };
    glBindTexture(textureTypeLut[slotIndx], m_textureSlots[slotIndx]);
  }

  void Renderer::SetShadowAtlas(TexturePtr shadowAtlas)
  {
    m_shadowAtlas = shadowAtlas;
  }

  CubeMapPtr Renderer::GenerateCubemapFrom2DTexture(TexturePtr texture,
                                                    uint width,
                                                    uint height,
                                                    float exposure)
  {
    const RenderTargetSettigs set = {0,
                                     GraphicTypes::TargetCubeMap,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::FormatRGBA16F,
                                     GraphicTypes::FormatRGBA,
                                     GraphicTypes::TypeFloat};

    RenderTargetPtr cubeMapRt =
        std::make_shared<RenderTarget>(width, height, set);
    cubeMapRt->Init();

    // Create material
    MaterialPtr mat = std::make_shared<Material>();
    ShaderPtr vert  = GetShaderManager()->Create<Shader>(
        ShaderPath("equirectToCubeVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("equirectToCubeFrag.shader", true));
    frag->m_shaderParams["Exposure"] = exposure;

    mat->m_diffuseTexture            = texture;
    mat->m_vertexShader              = vert;
    mat->m_fragmentShader            = frag;
    mat->GetRenderState()->cullMode  = CullingType::TwoSided;
    mat->Init();

    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({width, height, false, false});

    // Views for 6 different angles
    static Camera cam;
    cam.SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
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

      cam.m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
      cam.m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
      cam.m_node->SetScale(sca);

      m_utilFramebuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          cubeMapRt,
          0,
          -1,
          (Framebuffer::CubemapFace) i);

      SetFramebuffer(m_utilFramebuffer, true, Vec4(0.0f));
      DrawCube(&cam, mat);
    }
    SetFramebuffer(nullptr);

    // Take the ownership of render target.
    CubeMapPtr cubeMap     = std::make_shared<CubeMap>(cubeMapRt->m_textureId);
    cubeMapRt->m_textureId = 0;
    cubeMapRt              = nullptr;

    return cubeMap;
  }

  CubeMapPtr Renderer::GenerateEnvIrradianceMap(CubeMapPtr cubemap,
                                                uint width,
                                                uint height)
  {
    const RenderTargetSettigs set = {0,
                                     GraphicTypes::TargetCubeMap,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::FormatRGBA16F,
                                     GraphicTypes::FormatRGBA,
                                     GraphicTypes::TypeFloat};
    RenderTargetPtr cubeMapRt =
        std::make_shared<RenderTarget>(width, height, set);
    cubeMapRt->Init();

    // Views for 6 different angles
    CameraPtr cam = std::make_shared<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    // Create material
    MaterialPtr mat = std::make_shared<Material>();
    ShaderPtr vert  = GetShaderManager()->Create<Shader>(
        ShaderPath("irradianceGenerateVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("irradianceGenerateFrag.shader", true));

    mat->m_cubeMap                  = cubemap;
    mat->m_vertexShader             = vert;
    mat->m_fragmentShader           = frag;
    mat->GetRenderState()->cullMode = CullingType::TwoSided;
    mat->Init();

    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({width, height, false, false});

    for (int i = 0; i < 6; ++i)
    {
      Vec3 pos;
      Quaternion rot;
      Vec3 sca;
      DecomposeMatrix(views[i], &pos, &rot, &sca);

      cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
      cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
      cam->m_node->SetScale(sca);

      m_utilFramebuffer->SetAttachment(
          Framebuffer::Attachment::ColorAttachment0,
          cubeMapRt,
          0,
          -1,
          (Framebuffer::CubemapFace) i);

      SetFramebuffer(m_utilFramebuffer, true, Vec4(0.0f));
      DrawCube(cam.get(), mat);
    }
    SetFramebuffer(nullptr);

    // Take the ownership of render target.
    CubeMapPtr cubeMap     = std::make_shared<CubeMap>(cubeMapRt->m_textureId);
    cubeMapRt->m_textureId = 0;
    cubeMapRt              = nullptr;

    return cubeMap;
  }

  CubeMapPtr Renderer::GenerateEnvPrefilteredMap(CubeMapPtr cubemap,
                                                 uint width,
                                                 uint height,
                                                 int mipMaps)
  {
    const RenderTargetSettigs set = {0,
                                     GraphicTypes::TargetCubeMap,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::FormatRGBA16F,
                                     GraphicTypes::FormatRGBA,
                                     GraphicTypes::TypeFloat};
    RenderTargetPtr cubemapRt =
        std::make_shared<RenderTarget>(width, height, set);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapRt->m_textureId);
    cubemapRt->Init();

    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapRt->m_textureId);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_NEAREST);

    // Views for 6 different angles
    CameraPtr cam = std::make_shared<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    // Create material
    MaterialPtr mat = std::make_shared<Material>();
    ShaderPtr vert  = GetShaderManager()->Create<Shader>(
        ShaderPath("positionVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("preFilterEnvMapFrag.shader", true));

    mat->m_cubeMap                  = cubemap;
    mat->m_vertexShader             = vert;
    mat->m_fragmentShader           = frag;
    mat->GetRenderState()->cullMode = CullingType::TwoSided;
    mat->Init();

    // No need to re init framebuffer since m_util framebuffer has only 1
    // render target
    m_utilFramebuffer->Init({width, height, false, false});

    UVec2 lastViewportSize = m_viewportSize;

    for (int mip = 0; mip < mipMaps; ++mip)
    {
      for (int i = 0; i < 6; ++i)
      {
        Vec3 pos;
        Quaternion rot;
        Vec3 sca;
        DecomposeMatrix(views[i], &pos, &rot, &sca);

        cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
        cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
        cam->m_node->SetScale(sca);

        m_utilFramebuffer->SetAttachment(
            Framebuffer::Attachment::ColorAttachment0,
            cubemapRt,
            mip,
            -1,
            (Framebuffer::CubemapFace) i);

        uint w = (uint) (width * std::powf(0.5f, (float) mip));
        uint h = (uint) (height * std::powf(0.5f, (float) mip));

        frag->SetShaderParameter(
            "roughness",
            ParameterVariant((float) mip / (float) mipMaps));
        frag->SetShaderParameter("resPerFace", ParameterVariant((float) w));

        SetFramebuffer(m_utilFramebuffer, true, Vec4(0.0));
        SetViewportSize(w, h);

        DrawCube(cam.get(), mat);
      }
    }

    SetFramebuffer(nullptr);
    SetViewportSize(lastViewportSize.x, lastViewportSize.y);

    // Take the ownership of render target.
    CubeMapPtr cubeMap     = std::make_shared<CubeMap>(cubemapRt->m_textureId);
    cubemapRt->m_textureId = 0;
    cubemapRt              = nullptr;

    return cubeMap;
  }

  void Renderer::ResetTextureSlots()
  {
    for (int i = 0; i < 17; i++)
    {
      SetTexture(i, 0);
    }
  }

} // namespace ToolKit
