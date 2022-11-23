#include "Renderer.h"

#include "Camera.h"
#include "DirectionComponent.h"
#include "Drawable.h"
#include "GL/glew.h"
#include "Material.h"
#include "Mesh.h"
#include "Node.h"
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

#include <algorithm>
#include <random>

#include "DebugNew.h"

namespace ToolKit
{
#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

  Renderer::Renderer()
  {
    m_uiCamera        = new Camera();
    m_utilFramebuffer = std::make_shared<Framebuffer>();
  }

  Renderer::~Renderer()
  {
    SafeDel(m_uiCamera);
  }

  void Renderer::RenderScene(const ScenePtr scene,
                             Viewport* viewport,
                             const LightRawPtrArray& editorLights)
  {
    Camera* cam                = viewport->GetCamera();
    EntityRawPtrArray entities = scene->GetEntities();

    SkyBase* sky = scene->GetSky();
    if (sky != nullptr)
    {
      sky->Init();
    }

    RenderEntities(entities, cam, viewport, editorLights, sky);
  }

  void Renderer::GenerateKernelAndNoiseForSSAOSamples(Vec3Array& ssaoKernel,
                                                      Vec2Array& ssaoNoise)
  {
    // generate sample kernel
    // ----------------------
    auto lerp = [](float a, float b, float f) { return a + f * (b - a); };

    std::uniform_real_distribution<GLfloat> randomFloats(
        0.0, 1.0); // generates random floats between 0.0 and 1.0
    std::default_random_engine generator;
    if (ssaoKernel.size() == 0)
      for (unsigned int i = 0; i < 64; ++i)
      {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator) * 2.0 - 1.0,
                         randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
      }

    // generate noise texture
    // ----------------------
    if (ssaoNoise.size() == 0)
      for (unsigned int i = 0; i < 16; i++)
      {
        glm::vec2 noise(randomFloats(generator) * 2.0 - 1.0,
                        randomFloats(generator) * 2.0 - 1.0);
        ssaoNoise.push_back(noise);
      }
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

  void Renderer::GenerateSSAOTexture(const EntityRawPtrArray& entities,
                                     Viewport* viewport)
  {
    Camera* cam = viewport->GetCamera();

    RenderTargetSettigs rtSet;
    rtSet.WarpS = rtSet.WarpT = GraphicTypes::UVClampToEdge;
    rtSet.InternalFormat      = GraphicTypes::FormatRGBA16F;
    rtSet.Format              = GraphicTypes::FormatRGBA;
    rtSet.Type                = GraphicTypes::TypeFloat;

    if (!viewport->m_ssaoPosition)
      viewport->m_ssaoPosition = std::make_shared<RenderTarget>(
          (uint) viewport->m_wndContentAreaSize.x,
          (uint) viewport->m_wndContentAreaSize.y,
          rtSet);

    viewport->m_ssaoPosition->Init();
    viewport->m_ssaoPosition->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);

    if (!viewport->m_ssaoNormal)
      viewport->m_ssaoNormal = std::make_shared<RenderTarget>(
          (uint) viewport->m_wndContentAreaSize.x,
          (uint) viewport->m_wndContentAreaSize.y,
          rtSet);

    viewport->m_ssaoNormal->Init();
    viewport->m_ssaoNormal->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);

    if (!viewport->m_ssaoGBuffer)
      viewport->m_ssaoGBuffer = std::make_shared<Framebuffer>();

    viewport->m_ssaoGBuffer->Init({(uint) viewport->m_wndContentAreaSize.x,
                                   (uint) viewport->m_wndContentAreaSize.y,
                                   0,
                                   false,
                                   true});

    viewport->m_ssaoGBuffer->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);

    viewport->m_ssaoGBuffer->SetAttachment(
        Framebuffer::Attachment::ColorAttachment0, viewport->m_ssaoPosition);

    viewport->m_ssaoGBuffer->SetAttachment(
        Framebuffer::Attachment::ColorAttachment1, viewport->m_ssaoNormal);

    SetFramebuffer(viewport->m_ssaoGBuffer, true, {0.0f, 0.0f, 0.0f, 1.0});

    if (m_aoMat == nullptr)
    {
      ShaderPtr ssaoGeoVert = GetShaderManager()->Create<Shader>(
          ShaderPath("ssaoVertex.shader", true));
      ssaoGeoVert->Init();
      ssaoGeoVert->SetShaderParameter("viewMatrix",
                                      ParameterVariant(cam->GetViewMatrix()));
      ShaderPtr ssaoGeoFrag = GetShaderManager()->Create<Shader>(
          ShaderPath("ssaoGBufferFrag.shader", true));
      ssaoGeoFrag->Init();

      m_aoMat                   = std::make_shared<Material>();
      m_aoMat->m_vertexShader   = ssaoGeoVert;
      m_aoMat->m_fragmentShader = ssaoGeoFrag;
    }
    m_aoMat->UnInit();
    m_aoMat->m_fragmentShader->SetShaderParameter(
        "viewMatrix", ParameterVariant(cam->GetViewMatrix()));

    MaterialPtr overrideMatPrev = m_overrideMat;

    ClearBuffer(GraphicBitFields::ColorDepthBits);

    SetCameraLens(cam);

    for (Entity* ntt : entities)
    {
      if (ntt->IsDrawable() && ntt->GetVisibleVal())
      {
        MaterialPtr entityMat = ntt->GetRenderMaterial();
        if (!entityMat->GetRenderState()->AOInUse)
        {
          continue;
        }

        m_aoMat->m_alpha          = entityMat->m_alpha;
        m_aoMat->m_diffuseTexture = entityMat->m_diffuseTexture;

        m_overrideMat = m_aoMat;
        Render(ntt, cam);
      }
    }

    SetFramebuffer(nullptr);
    m_overrideMat = nullptr;

    static Vec3Array ssaoKernel;
    static Vec2Array ssaoNoise;
    GenerateKernelAndNoiseForSSAOSamples(ssaoKernel, ssaoNoise);

    static unsigned int noiseTexture = 0;
    if (noiseTexture == 0)
    {
      glGenTextures(1, &noiseTexture);
      glBindTexture(GL_TEXTURE_2D, noiseTexture);
      glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RG32F, 4, 4, 0, GL_RG, GL_FLOAT, &ssaoNoise[0]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    RenderTargetSettigs oneChannelSet = {};
    oneChannelSet.WarpS               = GraphicTypes::UVClampToEdge;
    oneChannelSet.WarpT               = GraphicTypes::UVClampToEdge;
    oneChannelSet.InternalFormat      = GraphicTypes::FormatR32F;
    oneChannelSet.Format              = GraphicTypes::FormatRed;
    oneChannelSet.Type                = GraphicTypes::TypeFloat;

    if (!viewport->m_ssao)
    {
      viewport->m_ssao = std::make_shared<RenderTarget>(
          (uint) viewport->m_wndContentAreaSize.x,
          (uint) viewport->m_wndContentAreaSize.y,
          oneChannelSet);
    }

    viewport->m_ssao->Init();
    viewport->m_ssao->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);

    if (!viewport->m_ssaoBuffer)
      viewport->m_ssaoBuffer = std::make_shared<Framebuffer>();
    viewport->m_ssaoBuffer->Init({(uint) viewport->m_wndContentAreaSize.x,
                                  (uint) viewport->m_wndContentAreaSize.y,
                                  0,
                                  false,
                                  true});

    viewport->m_ssaoBuffer->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);
    viewport->m_ssaoBuffer->SetAttachment(
        Framebuffer::Attachment::ColorAttachment0, viewport->m_ssao);
    SetFramebuffer(viewport->m_ssaoBuffer, true, {0.0f, 0.0f, 0.0f, 1.0});

    SetTexture(0, viewport->m_ssaoPosition->m_textureId);
    SetTexture(1, viewport->m_ssaoNormal->m_textureId);
    SetTexture(2, noiseTexture);

    if (!viewport->m_ssaoCalcMat)
    {
      viewport->m_ssaoCalcMat = std::make_shared<Material>();
      ShaderPtr ssaoVert      = GetShaderManager()->Create<Shader>(
          ShaderPath("ssaoCalcVert.shader", true));
      ShaderPtr ssaoFrag = GetShaderManager()->Create<Shader>(
          ShaderPath("ssaoCalcFrag.shader", true));
      viewport->m_ssaoCalcMat->m_vertexShader   = ssaoVert;
      viewport->m_ssaoCalcMat->m_fragmentShader = ssaoFrag;
      viewport->m_ssaoCalcMat->Init();
    }
    ShaderPtr ssaoFrag = viewport->m_ssaoCalcMat->m_fragmentShader;

    ssaoFrag->SetShaderParameter("projection",
                                 ParameterVariant(cam->GetProjectionMatrix()));
    ssaoFrag->SetShaderParameter(
        "screen_size", ParameterVariant(viewport->m_wndContentAreaSize));
    ssaoFrag->SetShaderParameter("gPosition", ParameterVariant(0));
    ssaoFrag->SetShaderParameter("gNormal", ParameterVariant(1));
    ssaoFrag->SetShaderParameter("texNoise", ParameterVariant(2));
    for (unsigned int i = 0; i < 64; ++i)
      ssaoFrag->SetShaderParameter(g_ssaoSamplesStrCache[i],
                                   ParameterVariant(ssaoKernel[i]));

    DrawFullQuad(viewport->m_ssaoCalcMat);
    SetFramebuffer(nullptr);

    if (!viewport->m_ssaoBlur)
      viewport->m_ssaoBlur = std::make_shared<RenderTarget>(
          (uint) viewport->m_wndContentAreaSize.x,
          (uint) viewport->m_wndContentAreaSize.y,
          oneChannelSet);
    viewport->m_ssaoBlur->Init();
    viewport->m_ssaoBlur->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);

    if (!viewport->m_ssaoBufferBlur)
      viewport->m_ssaoBufferBlur = std::make_shared<Framebuffer>();
    viewport->m_ssaoBufferBlur->Init({(uint) viewport->m_wndContentAreaSize.x,
                                      (uint) viewport->m_wndContentAreaSize.y,
                                      0,
                                      false,
                                      true});

    viewport->m_ssaoBufferBlur->ReconstructIfNeeded(
        (uint) viewport->m_wndContentAreaSize.x,
        (uint) viewport->m_wndContentAreaSize.y);

    const Vec2 scale = 1.0f / viewport->m_wndContentAreaSize;
    ApplyAverageBlur(viewport->m_ssao, viewport->m_ssaoBlur, X_AXIS, scale.x);
    ApplyAverageBlur(viewport->m_ssaoBlur, viewport->m_ssao, Y_AXIS, scale.y);

    m_overrideMat = overrideMatPrev;
    SetFramebuffer(nullptr);
    SetTexture(5, (uint) viewport->m_ssao->m_textureId);
    // Debug purpose.
    {
      // ShaderPtr quad = GetShaderManager()->Create<Shader>(
      //     ShaderPath("unlitFrag.shader", true));
      // quad->SetShaderParameter("s_texture0", ParameterVariant(5));
      // SetViewport(viewport);
      // DrawFullQuad(quad);
    }
  }

  /**
   * Renders given UILayer to given Viewport.
   * @param layer UILayer that will be rendered.
   * @param viewport that UILayer will be rendered with.
   */
  void Renderer::RenderUI(Viewport* viewport, UILayer* layer)
  {
    float halfWidth  = viewport->m_wndContentAreaSize.x * 0.5f;
    float halfHeight = viewport->m_wndContentAreaSize.y * 0.5f;

    m_uiCamera->SetLens(
        -halfWidth, halfWidth, -halfHeight, halfHeight, 0.5f, 1000.0f);

    EntityRawPtrArray entities = layer->m_scene->GetEntities();
    RenderEntities(entities, m_uiCamera, viewport);
  }

  void Renderer::Render(Entity* ntt,
                        Camera* cam,
                        const LightRawPtrArray& lights)
  {
    if (!cam->IsOrtographic())
    {
      // TODO: Orthographic mode must support environment lighting.
      FindEnvironmentLight(ntt);
    }

    MeshComponentPtrArray meshComponents;
    ntt->GetComponent<MeshComponent>(meshComponents);

    MaterialPtr nttMat;
    if (MaterialComponentPtr matCom = ntt->GetComponent<MaterialComponent>())
    {
      if (nttMat = matCom->GetMaterialVal())
      {
        nttMat->Init();
      }
    }

    MultiMaterialPtr mmComp = ntt->GetComponent<MultiMaterialComponent>();

    // Skeleton Component is used by all meshes of an entity.
    const auto& updateAndBindSkinningTextures = [ntt, this]() {
      SkeletonComponentPtr skelComp = ntt->GetComponent<SkeletonComponent>();
      if (skelComp == nullptr)
      {
        return;
      }

      SkeletonPtr skel = skelComp->GetSkeletonResourceVal();
      if (skel == nullptr)
      {
        return;
      }

      // Bind bone textures
      // This is valid because these slots will be used by every shader program
      //   below (Renderer::TextureSlot system).
      // But bone count can't be bound here because its location changes every
      //   shader program
      SetTexture(2, skel->m_bindPoseTexture->m_textureId);
      SetTexture(3, skelComp->m_map->boneTransformNodeTexture->m_textureId);

      skelComp->m_map->UpdateGPUTexture();
    };

    updateAndBindSkinningTextures();

    for (MeshComponentPtr meshCom : meshComponents)
    {
      MeshPtr mainMesh = meshCom->GetMeshVal();
      m_lights         = GetBestLights(ntt, lights);
      m_cam            = cam;
      SetProjectViewModel(ntt, cam);

      mainMesh->Init();

      MeshRawPtrArray meshCollector;
      mainMesh->GetAllMeshes(meshCollector);

      for (uint meshIndx = 0; meshIndx < meshCollector.size(); meshIndx++)
      {
        Mesh* mesh = meshCollector[meshIndx];

        if (mesh->m_vertexCount == 0)
        {
          continue;
        }

        if (mmComp && mmComp->GetMaterialList().size() > meshIndx)
        {
          nttMat = mmComp->GetMaterialList()[meshIndx];
          nttMat->Init();
        }
        mesh->Init();
        if (m_overrideMat != nullptr)
        {
          m_mat = m_overrideMat;
        }
        else
        {
          m_mat = nttMat ? nttMat : mesh->m_material;
        }

        ProgramPtr prg =
            CreateProgram(m_mat->m_vertexShader, m_mat->m_fragmentShader);

        BindProgram(prg);

        auto activateSkinning = [prg, ntt](uint isSkinned) {
          GLint isSkinnedLoc = glGetUniformLocation(prg->m_handle, "isSkinned");
          glUniform1ui(isSkinnedLoc, isSkinned);
          if (isSkinned)
          {
            GLint numBonesLoc = glGetUniformLocation(prg->m_handle, "numBones");
            float boneCount =
                static_cast<float>(ntt->GetComponent<SkeletonComponent>()
                                       ->GetSkeletonResourceVal()
                                       ->m_bones.size());
            glUniform1fv(numBonesLoc, 1, &boneCount);
          }
        };
        activateSkinning(mesh->IsSkinned());

        RenderState rs = *m_mat->GetRenderState();
        if (m_overrideDiffuseTexture && m_overrideMat)
        {
          Material* secondaryMat =
              nttMat ? nttMat.get() : mesh->m_material.get();
          if (secondaryMat->m_diffuseTexture)
          {
            rs.diffuseTexture = secondaryMat->m_diffuseTexture->m_textureId;
          }
        }

        SetRenderState(&rs, prg);
        FeedUniforms(prg);

        glBindVertexArray(mesh->m_vaoId);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->m_vboVertexId);
        SetVertexLayout(mesh->m_vertexLayout);

        if (mesh->m_indexCount != 0)
        {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_vboIndexId);
          glDrawElements((GLenum) rs.drawType,
                         mesh->m_indexCount,
                         GL_UNSIGNED_INT,
                         nullptr);
        }
        else
        {
          glDrawArrays((GLenum) rs.drawType, 0, mesh->m_vertexCount);
        }
      }
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
    SetRenderState(rs, prog);

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
    SetVertexLayout(VertexLayout::Mesh);

    glDrawArrays((GLenum) rs->drawType, 0, mesh->m_vertexCount);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    SetVertexLayout(VertexLayout::None);
  }

  void Renderer::Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions)
  {
    Surface* surface = object->GetCurrentSurface();

    Node* backup    = surface->m_node;
    surface->m_node = object->m_node;

    Render2d(surface, screenDimensions);

    surface->m_node = backup;
  }

  void Renderer::SetRenderState(const RenderState* const state,
                                ProgramPtr program)
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

    if (m_renderState.depthTestEnabled != state->depthTestEnabled)
    {
      if (state->depthTestEnabled)
      {
        glEnable(GL_DEPTH_TEST);
      }
      else
      {
        glDisable(GL_DEPTH_TEST);
      }
      m_renderState.depthTestEnabled = state->depthTestEnabled;
    }

    if (m_renderState.blendFunction != state->blendFunction)
    {
      switch (state->blendFunction)
      {
      case BlendFunction::NONE:
        glDisable(GL_BLEND);
        break;
      case BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
      }

      m_renderState.blendFunction = state->blendFunction;
    }

    if (state->diffuseTextureInUse)
    {
      m_renderState.diffuseTexture      = state->diffuseTexture;
      m_renderState.diffuseTextureInUse = state->diffuseTextureInUse;
      SetTexture(0, state->diffuseTexture);
    }

    if (state->cubeMapInUse)
    {
      m_renderState.cubeMap      = state->cubeMap;
      m_renderState.cubeMapInUse = state->cubeMapInUse;
      SetTexture(6, state->cubeMap);
    }

    if (m_renderState.lineWidth != state->lineWidth)
    {
      m_renderState.lineWidth = state->lineWidth;
      glLineWidth(m_renderState.lineWidth);
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
    if (fb != nullptr)
    {
      if (m_framebuffer)
      {
        if (fb->GetFboId() == m_framebuffer->GetFboId())
        {
          return;
        }
      }

      glBindFramebuffer(GL_FRAMEBUFFER, fb->GetFboId());

      FramebufferSettings fbSet = fb->GetSettings();
      SetViewportSize(fbSet.width, fbSet.height);
    }
    else
    {
      // Set backbuffer as draw area.
      m_framebuffer = nullptr;
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      SetViewportSize(m_windowSize.x, m_windowSize.y);
    }

    if (clear)
    {
      ClearBuffer(GraphicBitFields::DepthStencilBits);
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

  FramebufferPtr Renderer::GetFrameBuffer()
  {
    return m_framebuffer;
  }

  void Renderer::ClearFrameBuffer(FramebufferPtr fb, const Vec4& color)
  {
    SwapFramebuffer(fb, true, color);
    SwapFramebuffer(fb, false);
  }

  void Renderer::ClearColorBuffer(const Vec4& value)
  {
    glClearColor(value.r, value.g, value.b, value.a);
    glClear((GLbitfield) GraphicBitFields::ColorBits);
  }

  void Renderer::ClearBuffer(GraphicBitFields fields)
  {
    glClearColor(
        m_clearColor.r, m_clearColor.g, m_clearColor.b, m_clearColor.a);
    glClear((GLbitfield) fields);
  }

  void Renderer::ColorMask(bool r, bool g, bool b, bool a)
  {
    glColorMask(r, g, b, a);
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
    static Quad quad;
    quad.GetMeshComponent()->GetMeshVal()->m_material = mat;

    static Camera quadCam;
    Render(&quad, &quadCam);
  }

  void Renderer::DrawCube(Camera* cam, MaterialPtr mat, const Mat4& transform)
  {
    static Cube cube;
    cube.Generate();
    cube.m_node->SetTransform(transform);

    MaterialComponentPtr matc = cube.GetMaterialComponent();
    if (matc == nullptr)
    {
      cube.AddComponent(new MaterialComponent);
    }
    cube.GetMaterialComponent()->SetMaterialVal(mat);

    Render(&cube, cam);
  }

  void Renderer::RenderEntities(EntityRawPtrArray& entities,
                                Camera* cam,
                                Viewport* viewport,
                                const LightRawPtrArray& lights,
                                SkyBase* sky)
  {
    ShadowPass(lights, entities);

    CollectEnvironmentVolumes(entities);

    // Dropout non visible & drawable entities.
    entities.erase(std::remove_if(entities.begin(),
                                  entities.end(),
                                  [](Entity* ntt) -> bool {
                                    return !ntt->GetVisibleVal() ||
                                           !ntt->IsDrawable();
                                  }),
                   entities.end());

    FrustumCull(entities, cam);

    // Update billboards.
    for (Entity* ntt : entities)
    {
      if (ntt->GetType() == EntityType::Entity_Billboard)
      {
        Billboard* billboard = static_cast<Billboard*>(ntt);
        billboard->LookAt(cam, viewport->GetBillboardScale());
      }
    }

    // GenerateSSAOTexture(entities, viewport);

    SetViewport(viewport);

    SetCameraLens(cam);

    if (sky && !cam->IsOrtographic())
    {
      RenderSky(sky, cam);
    }

    EntityRawPtrArray blendedEntities;
    GetTransparentEntites(entities, blendedEntities);

    RenderOpaque(entities, cam, lights);

    RenderTransparent(blendedEntities, cam, lights);
  }

  void Renderer::GetTransparentEntites(EntityRawPtrArray& entities,
                                       EntityRawPtrArray& blendedEntities)
  {
    auto delTrFn = [&blendedEntities](Entity* ntt) -> bool {
      // Check too see if there are any material with blend state.
      MaterialComponentPtrArray materials;
      ntt->GetComponent<MaterialComponent>(materials);

      if (!materials.empty())
      {
        for (MaterialComponentPtr& mt : materials)
        {
          if (mt->GetMaterialVal() &&
              mt->GetMaterialVal()->GetRenderState()->blendFunction !=
                  BlendFunction::NONE)
          {
            blendedEntities.push_back(ntt);
            return true;
          }
        }
      }
      else
      {
        MeshComponentPtrArray meshes;
        ntt->GetComponent<MeshComponent>(meshes);

        if (meshes.empty())
        {
          return false;
        }

        for (MeshComponentPtr& ms : meshes)
        {
          MeshRawCPtrArray all;
          ms->GetMeshVal()->GetAllMeshes(all);
          for (const Mesh* m : all)
          {
            if (m->m_material->GetRenderState()->blendFunction !=
                BlendFunction::NONE)
            {
              blendedEntities.push_back(ntt);
              return true;
            }
          }
        }
      }

      return false;
    };

    entities.erase(std::remove_if(entities.begin(), entities.end(), delTrFn),
                   entities.end());
  }

  void Renderer::RenderOpaque(EntityRawPtrArray entities,
                              Camera* cam,
                              const LightRawPtrArray& editorLights)
  {
    // Render opaque objects
    for (Entity* ntt : entities)
    {
      Render(ntt, cam, editorLights);
    }
  }

  void Renderer::RenderTransparent(EntityRawPtrArray entities,
                                   Camera* cam,
                                   const LightRawPtrArray& editorLights)
  {
    StableSortByDistanceToCamera(entities, cam);
    StableSortByMaterialPriority(entities);

    // Render transparent entities
    for (Entity* ntt : entities)
    {
      // For two sided materials,
      // first render back of transparent objects then render front
      MaterialPtr renderMaterial = GetRenderMaterial(ntt);
      if (renderMaterial->GetRenderState()->cullMode == CullingType::TwoSided)
      {
        renderMaterial->GetRenderState()->cullMode = CullingType::Front;
        Render(ntt, cam, editorLights);

        renderMaterial->GetRenderState()->cullMode = CullingType::Back;
        Render(ntt, cam, editorLights);

        renderMaterial->GetRenderState()->cullMode = CullingType::TwoSided;
      }
      else
      {
        Render(ntt, cam, editorLights);
      }
    }
  }

  MaterialPtr Renderer::GetRenderMaterial(Entity* entity)
  {
    if (m_overrideMat)
    {
      return m_overrideMat;
    }

    return entity->GetRenderMaterial();
  }

  void Renderer::RenderSky(SkyBase* sky, Camera* cam)
  {
    if (sky == nullptr || (!sky->GetDrawSkyVal()))
    {
      return;
    }

    glDepthFunc(GL_LEQUAL);

    MaterialPtr skyboxMat = sky->GetSkyboxMaterial();
    const Mat4 rotation =
        Mat4(sky->m_node->GetOrientation(TransformationSpace::TS_WORLD));
    DrawCube(cam, skyboxMat, rotation);

    glDepthFunc(GL_LESS); // Return to default depth test
  }

  // An interval has start time and end time
  struct LightSortStruct
  {
    Light* light        = nullptr;
    uint intersectCount = 0;
  };

  // Compares two intervals according to starting times.
  bool CompareLightIntersects(const LightSortStruct& i1,
                              const LightSortStruct& i2)
  {
    return (i1.intersectCount > i2.intersectCount);
  }

  LightRawPtrArray Renderer::GetBestLights(Entity* entity,
                                           const LightRawPtrArray& lights)
  {
    LightRawPtrArray bestLights;
    bestLights.reserve(lights.size());

    // Find the end of directional lights
    for (int i = 0; i < lights.size(); i++)
    {
      if (lights[i]->GetType() == EntityType::Entity_DirectionalLight)
      {
        bestLights.push_back(lights[i]);
      }
    }

    // Add the lights inside of the radius first
    std::vector<LightSortStruct> intersectCounts(lights.size());
    BoundingBox aabb = entity->GetAABB(true);
    for (uint lightIndx = 0; lightIndx < lights.size(); lightIndx++)
    {
      float radius;
      Light* light = lights[lightIndx];
      if (light->GetType() == EntityType::Entity_PointLight)
      {
        radius = static_cast<PointLight*>(light)->GetRadiusVal();
      }
      else if (light->GetType() == EntityType::Entity_SpotLight)
      {
        radius = static_cast<SpotLight*>(light)->GetRadiusVal();
      }
      else
      {
        continue;
      }

      intersectCounts[lightIndx].light = light;
      uint& curIntersectCount = intersectCounts[lightIndx].intersectCount;

      /* This algorithms can be used for better sorting
      for (uint dimIndx = 0; dimIndx < 3; dimIndx++)
      {
        for (uint isMin = 0; isMin < 2; isMin++)
        {
          Vec3 p     = aabb.min;
          p[dimIndx] = (isMin == 0) ? aabb.min[dimIndx] : aabb.max[dimIndx];
          float dist = glm::length(
              p - light->m_node->GetTranslation(TransformationSpace::TS_WORLD));
          if (dist <= radius)
          {
            curIntersectCount++;
          }
        }
      }*/

      if (light->GetType() == EntityType::Entity_SpotLight)
      {
        light->UpdateShadowCamera();

        Frustum spotFrustum =
            ExtractFrustum(light->m_shadowMapCameraProjectionViewMatrix, false);

        if (FrustumBoxIntersection(spotFrustum, aabb) !=
            IntersectResult::Outside)
        {
          curIntersectCount++;
        }
      }
      if (light->GetType() == EntityType::Entity_PointLight)
      {
        BoundingSphere lightSphere = {light->m_node->GetTranslation(), radius};
        if (SphereBoxIntersection(lightSphere, aabb))
        {
          curIntersectCount++;
        }
      }
    }

    std::sort(
        intersectCounts.begin(), intersectCounts.end(), CompareLightIntersects);

    for (uint i = 0; i < intersectCounts.size(); i++)
    {
      if (intersectCounts[i].intersectCount == 0)
      {
        break;
      }
      bestLights.push_back(intersectCounts[i].light);
    }

    return bestLights;
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
          {(uint) source->m_width, (uint) source->m_height, 0, false, false});
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

  void Renderer::ToggleBlending(bool blending)
  {
    if (blending)
    {
      glEnable(GL_BLEND);
    }
    else
    {
      glDisable(GL_BLEND);
    }
  }

  void Renderer::CollectEnvironmentVolumes(const EntityRawPtrArray& entities)
  {
    // Find entities which have environment component
    m_environmentLightEntities.clear();
    for (Entity* ntt : entities)
    {
      if (ntt->GetType() == EntityType::Entity_Sky)
      {
        continue;
      }

      EnvironmentComponentPtr envCom =
          ntt->GetComponent<EnvironmentComponent>();
      if (envCom != nullptr && envCom->GetHdriVal() != nullptr &&
          envCom->GetHdriVal()->IsTextureAssigned() &&
          envCom->GetIlluminateVal())
      {
        envCom->Init(true);
        m_environmentLightEntities.push_back(ntt);
      }
    }
  }

  void Renderer::FindEnvironmentLight(Entity* entity)
  {
    // Note: If multiple bounding boxes are intersecting and the intersection
    // volume includes the entity, the smaller bounding box is taken

    // Iterate all entities and mark the ones which should
    // be lit with environment light

    Entity* env     = nullptr;
    MaterialPtr mat = GetRenderMaterial(entity);

    Vec3 pos = entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    BoundingBox bestBox{ZERO, ZERO};
    BoundingBox currentBox;
    for (Entity* envNtt : m_environmentLightEntities)
    {
      currentBox = envNtt->GetComponent<EnvironmentComponent>()->GetBBox();

      if (PointInsideBBox(pos, currentBox.max, currentBox.min))
      {
        auto setCurrentBBox = [&bestBox, &env](const BoundingBox& box,
                                               Entity* ntt) -> void {
          bestBox = box;
          env     = ntt;
        };

        if (bestBox.max == bestBox.min && bestBox.max == ZERO)
        {
          setCurrentBBox(currentBox, envNtt);
          continue;
        }

        bool change = false;
        if (BoxBoxIntersection(bestBox, currentBox))
        {
          // Take the smaller box
          if (bestBox.Volume() > currentBox.Volume())
          {
            change = true;
          }
        }
        else
        {
          change = true;
        }

        if (change)
        {
          setCurrentBBox(currentBox, envNtt);
        }
      }
    }

    if (env != nullptr)
    {
      mat->GetRenderState()->IBLInUse = true;
      EnvironmentComponentPtr envCom =
          env->GetComponent<EnvironmentComponent>();
      mat->GetRenderState()->iblIntensity = envCom->GetIntensityVal();
      if (CubeMapPtr irradianceCubemap =
              envCom->GetHdriVal()->GetIrradianceCubemap())
      {
        mat->GetRenderState()->irradianceMap = irradianceCubemap->m_textureId;
      }
      m_iblRotation =
          Mat4(env->m_node->GetOrientation(TransformationSpace::TS_WORLD));
    }
    else
    {
      // Sky light
      SkyBase* sky = GetSceneManager()->GetCurrentScene()->GetSky();
      if (sky != nullptr && sky->IsInitialized() && sky->GetIlluminateVal())
      {
        mat->GetRenderState()->IBLInUse = true;
        if (sky->GetType() == EntityType::Entity_Sky)
        {
          if (CubeMapPtr irradianceCubemap =
                  static_cast<Sky*>(sky)
                      ->GetComponent<EnvironmentComponent>()
                      ->GetHdriVal()
                      ->GetIrradianceCubemap())
          {
            mat->GetRenderState()->irradianceMap =
                irradianceCubemap->m_textureId;
          }
        }
        else if (sky->GetType() == EntityType::Entity_GradientSky)
        {
          mat->GetRenderState()->irradianceMap =
              static_cast<GradientSky*>(sky)->GetIrradianceMap()->m_textureId;
        }
        mat->GetRenderState()->iblIntensity = sky->GetIntensityVal();
        m_iblRotation =
            Mat4(sky->m_node->GetOrientation(TransformationSpace::TS_WORLD));
      }
      else
      {
        mat->GetRenderState()->IBLInUse      = false;
        mat->GetRenderState()->irradianceMap = 0;
        m_iblRotation                        = Mat4(1.0f);
      }
    }
  }

  void Renderer::ShadowPass(const LightRawPtrArray& lights,
                            const EntityRawPtrArray& entities)
  {
    UpdateShadowMaps(lights, entities);
    FilterShadowMaps(lights);
  }

  void Renderer::UpdateShadowMaps(const LightRawPtrArray& lights,
                                  const EntityRawPtrArray& entities)
  {
    MaterialPtr lastOverrideMaterial = m_overrideMat;

    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    for (Light* light : lights)
    {
      // Update shadow map ProjView matrix every frame for all lights
      if (light->GetCastShadowVal())
      {
        if (light->GetCastShadowVal() == false)
        {
          continue;
        }

        if (light->GetType() == EntityType::Entity_DirectionalLight)
        {
          static_cast<DirectionalLight*>(light)->UpdateShadowFrustum(entities);
        }
        else
        {
          light->UpdateShadowCamera();
        }

        // Create framebuffer
        light->InitShadowMap();

        auto renderForShadowMapFn = [this](Light* light,
                                           EntityRawPtrArray entities) -> void {
          FrustumCull(entities, light->m_shadowCamera);
          ClearBuffer(GraphicBitFields::DepthBits);
          m_overrideMat = light->GetShadowMaterial();
          for (Entity* ntt : entities)
          {
            if (ntt->IsDrawable() &&
                ntt->GetMeshComponent()->GetCastShadowVal())
            {
              MaterialPtr entityMat = ntt->GetRenderMaterial();
              m_overrideMat->SetRenderState(entityMat->GetRenderState());
              m_overrideMat->UnInit();
              m_overrideMat->m_alpha          = entityMat->m_alpha;
              m_overrideMat->m_diffuseTexture = entityMat->m_diffuseTexture;
              m_overrideMat->GetRenderState()->blendFunction =
                  BlendFunction::NONE;
              m_overrideMat->Init();
              Render(ntt, light->m_shadowCamera);
            }
          }
        };

        switch (light->GetType())
        {
        case EntityType::Entity_PointLight: {
          // Initialize point light view transforms.
          static Quaternion rotations[6];
          static Vec3 scales[6];
          static bool viewsCalculated = false;
          if (!viewsCalculated)
          {
            Mat4 views[6] = {
                glm::lookAt(
                    ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(
                    ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(
                    ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
                glm::lookAt(
                    ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
                glm::lookAt(
                    ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
                glm::lookAt(
                    ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

            for (int i = 0; i < 6; ++i)
            {
              DecomposeMatrix(views[i], nullptr, &rotations[i], &scales[i]);
            }

            viewsCalculated = true;
          }

          FramebufferPtr smBuffer = light->GetShadowMapFramebuffer();
          SetFramebuffer(smBuffer, true, Vec4(1.0f));

          Vec2 shadowRes = light->GetShadowResolutionVal();
          glViewport(0, 0, uint(shadowRes.x), uint(shadowRes.y));

          for (unsigned int i = 0; i < 6; ++i)
          {
            smBuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                    light->GetShadowMapRenderTarget(),
                                    (Framebuffer::CubemapFace) i);

            light->m_node->SetOrientation(rotations[i]);

            // TODO: Scales are not needed. Remove.
            light->m_node->SetScale(scales[i]);

            renderForShadowMapFn(light, entities);
          }
        }
        case EntityType::Entity_DirectionalLight:
        case EntityType::Entity_SpotLight:
          SetFramebuffer(light->GetShadowMapFramebuffer(), true, Vec4(1.0f));
          renderForShadowMapFn(light, entities);
          break;
        default:
          break;
        }
      }
    }

    m_overrideMat = lastOverrideMaterial;
    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  void Renderer::FilterShadowMaps(const LightRawPtrArray& lights)
  {
    for (Light* light : lights)
    {
      if (!light->GetCastShadowVal() || light->GetShadowThicknessVal() < 0.001f)
      {
        continue;
      }

      if (light->GetType() == EntityType::Entity_PointLight)
      {
        continue;
      }
      const float softness = light->GetShadowThicknessVal();
      Apply7x1GaussianBlur(light->GetShadowMapRenderTarget(),
                           light->GetShadowMapTempBlurRt(),
                           X_AXIS,
                           softness / light->GetShadowResolutionVal().x);
      Apply7x1GaussianBlur(light->GetShadowMapTempBlurRt(),
                           light->GetShadowMapRenderTarget(),
                           Y_AXIS,
                           softness / light->GetShadowResolutionVal().y);
    }
  }

  void Renderer::Apply7x1GaussianBlur(const TexturePtr source,
                                      RenderTargetPtr dest,
                                      const Vec3& axis,
                                      const float amount)
  {
    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({0, 0, 0, false, false});

    if (m_gaussianBlurMaterial == nullptr)
    {
      ShaderPtr vert = GetShaderManager()->Create<Shader>(
          ShaderPath("gausBlur7x1Vert.shader", true));
      ShaderPtr frag = GetShaderManager()->Create<Shader>(
          ShaderPath("gausBlur7x1Frag.shader", true));
      m_gaussianBlurMaterial                   = std::make_shared<Material>();
      m_gaussianBlurMaterial->m_vertexShader   = vert;
      m_gaussianBlurMaterial->m_fragmentShader = frag;
    }

    m_gaussianBlurMaterial->UnInit();
    m_gaussianBlurMaterial->m_diffuseTexture = source;
    m_gaussianBlurMaterial->m_fragmentShader->SetShaderParameter(
        "BlurScale", ParameterVariant(axis * amount));
    m_gaussianBlurMaterial->Init();

    m_utilFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                     dest);

    SetFramebuffer(m_utilFramebuffer, true, Vec4(1.0f));
    DrawFullQuad(m_gaussianBlurMaterial);
  }

  void Renderer::ApplyAverageBlur(const TexturePtr source,
                                  RenderTargetPtr dest,
                                  const Vec3& axis,
                                  const float amount)
  {
    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({0, 0, 0, false, false});

    if (m_averageBlurMaterial == nullptr)
    {
      ShaderPtr vert = GetShaderManager()->Create<Shader>(
          ShaderPath("avgBlurVert.shader", true));
      ShaderPtr frag = GetShaderManager()->Create<Shader>(
          ShaderPath("avgBlurFrag.shader", true));
      m_averageBlurMaterial                   = std::make_shared<Material>();
      m_averageBlurMaterial->m_vertexShader   = vert;
      m_averageBlurMaterial->m_fragmentShader = frag;
    }

    m_averageBlurMaterial->UnInit();
    m_averageBlurMaterial->m_diffuseTexture = source;
    m_averageBlurMaterial->m_fragmentShader->SetShaderParameter(
        "BlurScale", ParameterVariant(axis * amount));
    m_averageBlurMaterial->Init();

    m_utilFramebuffer->SetAttachment(Framebuffer::Attachment::ColorAttachment0,
                                     dest);

    SetFramebuffer(m_utilFramebuffer, true, Vec4(1.0f));
    DrawFullQuad(m_averageBlurMaterial);
  }

  void Renderer::SetProjectViewModel(Entity* ntt, Camera* cam)
  {
    m_view = cam->GetViewMatrix();

    // Recalculate the projection matrix due to aspect ratio changes of the
    // current frame buffer.
    /*if (cam->IsOrtographic())
    {
      // ASPECT ??
      cam->SetLens(cam->Left(),
                   cam->Right(),
                   cam->Top(),
                   cam->Bottom(),
                   cam->Near(),
                   cam->Far());
    }
    else
    {
      float aspect = (float) m_viewportSize.x / (float) m_viewportSize.y;
      cam->SetLens(cam->Fov(), aspect);
    }*/

    m_project = cam->GetProjectionMatrix();
    m_model   = ntt->m_node->GetTransform(TransformationSpace::TS_WORLD);
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
      LinkProgram(
          program->m_handle, vertex->m_shaderHandle, fragment->m_shaderHandle);
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
        case Uniform::PROJECT_MODEL_VIEW: {
          GLint loc =
              glGetUniformLocation(program->m_handle, "ProjectViewModel");
          Mat4 mul = m_project * m_view * m_model;
          glUniformMatrix4fv(loc, 1, false, &mul[0][0]);
        }
        break;
        case Uniform::VIEW: {
          GLint loc = glGetUniformLocation(program->m_handle, "View");
          glUniformMatrix4fv(loc, 1, false, &m_view[0][0]);
        }
        case Uniform::MODEL: {
          GLint loc = glGetUniformLocation(program->m_handle, "Model");
          glUniformMatrix4fv(loc, 1, false, &m_model[0][0]);
        }
        break;
        case Uniform::INV_TR_MODEL: {
          GLint loc =
              glGetUniformLocation(program->m_handle, "InverseTransModel");
          Mat4 invTrModel = glm::transpose(glm::inverse(m_model));
          glUniformMatrix4fv(loc, 1, false, &invTrModel[0][0]);
        }
        break;
        case Uniform::LIGHT_DATA: {
          FeedLightUniforms(program);
        }
        break;
        case Uniform::CAM_DATA: {
          if (m_cam == nullptr)
            break;

          Camera::CamData data = m_cam->GetData();
          GLint loc = glGetUniformLocation(program->m_handle, "CamData.pos");
          glUniform3fv(loc, 1, &data.pos.x);
          loc = glGetUniformLocation(program->m_handle, "CamData.dir");
          glUniform3fv(loc, 1, &data.dir.x);
          loc = glGetUniformLocation(program->m_handle, "CamData.far");
          glUniform1f(loc, data.far);
        }
        break;
        case Uniform::COLOR: {
          if (m_mat == nullptr)
            break;

          Vec4 color = Vec4(m_mat->m_color, m_mat->m_alpha);
          if (m_mat->GetRenderState()->blendFunction !=
              BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA)
          {
            color.a = 1.0f;
          }

          GLint loc = glGetUniformLocation(program->m_handle, "Color");
          if (m_renderOnlyLighting)
          {
            const Vec4 overrideColor = Vec4(1.0f, 1.0f, 1.0f, color.a);
            glUniform4fv(loc, 1, &overrideColor.x);
          }
          else
          {
            glUniform4fv(loc, 1, &color.x);
          }
        }
        break;
        case Uniform::FRAME_COUNT: {
          GLint loc = glGetUniformLocation(program->m_handle, "FrameCount");
          glUniform1ui(loc, m_frameCount);
        }
        break;
        case Uniform::EXPOSURE: {
          GLint loc = glGetUniformLocation(program->m_handle, "Exposure");
          glUniform1f(loc, shader->m_shaderParams["Exposure"].GetVar<float>());
        }
        break;
        case Uniform::PROJECTION_VIEW_NO_TR: {
          GLint loc =
              glGetUniformLocation(program->m_handle, "ProjectionViewNoTr");
          // Zero transalate variables in model matrix
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
        case Uniform::USE_IBL: {
          m_renderState.IBLInUse = m_mat->GetRenderState()->IBLInUse;
          GLint loc = glGetUniformLocation(program->m_handle, "UseIbl");
          glUniform1f(loc, static_cast<float>(m_renderState.IBLInUse));
        }
        break;
        case Uniform::IBL_INTENSITY: {
          m_renderState.iblIntensity = m_mat->GetRenderState()->iblIntensity;
          GLint loc = glGetUniformLocation(program->m_handle, "IblIntensity");
          glUniform1f(loc, static_cast<float>(m_renderState.iblIntensity));
        }
        break;
        case Uniform::IBL_IRRADIANCE: {
          m_renderState.irradianceMap = m_mat->GetRenderState()->irradianceMap;
          SetTexture(7, m_renderState.irradianceMap);
        }
        break;
        case Uniform::DIFFUSE_TEXTURE_IN_USE: {
          GLint loc =
              glGetUniformLocation(program->m_handle, "DiffuseTextureInUse");
          glUniform1i(loc, (int) m_mat->GetRenderState()->diffuseTextureInUse);
        }
        break;
        case Uniform::COLOR_ALPHA: {
          if (m_mat == nullptr)
            break;

          GLint loc = glGetUniformLocation(program->m_handle, "colorAlpha");
          glUniform1f(loc, m_mat->m_alpha);
        }
        break;
        case Uniform::USE_AO: {
          m_renderState.AOInUse = m_mat->GetRenderState()->AOInUse;
          GLint loc = glGetUniformLocation(program->m_handle, "UseAO");
          glUniform1i(loc, (int) m_renderState.AOInUse);
        }
        break;
        case Uniform::IBL_ROTATION: {
          GLint loc = glGetUniformLocation(program->m_handle, "IblRotation");

          glUniformMatrix4fv(loc, 1, false, &m_iblRotation[0][0]);
        }
        break;
        case Uniform::LIGHTING_ONLY: {
          GLint loc = glGetUniformLocation(program->m_handle, "LightingOnly");
          glUniform1i(loc, m_renderOnlyLighting ? 1 : 0);
        }
        break;
        default:
          assert(false);
          break;
        }
      }

      // Custom variables.
      for (auto var : shader->m_shaderParams)
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
        case ParameterVariant::VariantType::Vec2:
          glUniform2fv(
              loc, 1, reinterpret_cast<float*>(&var.second.GetVar<Vec2>()));
          break;
        case ParameterVariant::VariantType::Vec3:
          glUniform3fv(
              loc, 1, reinterpret_cast<float*>(&var.second.GetVar<Vec3>()));
          break;
        case ParameterVariant::VariantType::Vec4:
          glUniform4fv(
              loc, 1, reinterpret_cast<float*>(&var.second.GetVar<Vec4>()));
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
    ResetShadowMapBindings(program);

    size_t lightSize =
        glm::min(m_lights.size(), m_rhiSettings::maxLightsPerObject);
    for (size_t i = 0; i < lightSize; i++)
    {
      Light* currLight = m_lights[i];

      EntityType type = currLight->GetType();

      // Point light uniforms
      if (type == EntityType::Entity_PointLight)
      {
        Vec3 color      = currLight->GetColorVal();
        float intensity = currLight->GetIntensityVal();
        Vec3 pos =
            currLight->m_node->GetTranslation(TransformationSpace::TS_WORLD);
        float radius = static_cast<PointLight*>(currLight)->GetRadiusVal();

        GLuint loc = glGetUniformLocation(program->m_handle,
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
            program->m_handle, g_lightprojectionViewMatrixStrCache[i].c_str());
        glUniformMatrix4fv(
            loc,
            1,
            GL_FALSE,
            &(currLight->m_shadowMapCameraProjectionViewMatrix)[0][0]);

        loc = glGetUniformLocation(
            program->m_handle, g_lightShadowMapCameraFarStrCache[i].c_str());
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

        if (FramebufferPtr shadowFrameBuffer =
                currLight->GetShadowMapFramebuffer())
        {
          SetShadowMapTexture(
              type,
              shadowFrameBuffer
                  ->GetAttachment(Framebuffer::Attachment::ColorAttachment0)
                  ->m_textureId,
              program);
        }
        else
        {
          GetLogger()->WriteConsole(
              LogType::Error,
              "Uninitilized shadow buffer ! Light Name: %s ID: %d",
              currLight->GetNameVal().c_str(),
              currLight->GetIdVal());
        }
      }
      GLuint loc = glGetUniformLocation(program->m_handle,
                                        g_lightCastShadowStrCache[i].c_str());
      glUniform1i(loc, static_cast<int>(castShadow));
    }

    GLint loc =
        glGetUniformLocation(program->m_handle, "LightData.activeCount");
    glUniform1i(loc, static_cast<int>(m_lights.size()));
  }

  void Renderer::SetVertexLayout(VertexLayout layout)
  {
    if (m_renderState.vertexLayout == layout)
    {
      return;
    }

    if (layout == VertexLayout::None)
    {
      for (int i = 0; i < 6; i++)
      {
        glDisableVertexAttribArray(i);
      }
    }

    if (layout == VertexLayout::Mesh)
    {
      GLuint offset = 0;
      glEnableVertexAttribArray(0); // Vertex
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(1); // Normal
      glVertexAttribPointer(
          1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(2); // Texture
      glVertexAttribPointer(
          2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
      offset += 2 * sizeof(float);

      glEnableVertexAttribArray(3); // BiTangent
      glVertexAttribPointer(
          3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), BUFFER_OFFSET(offset));
    }

    if (layout == VertexLayout::SkinMesh)
    {
      GLuint offset = 0;
      glEnableVertexAttribArray(0); // Vertex
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), 0);
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(1); // Normal
      glVertexAttribPointer(
          1, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 3 * sizeof(float);

      glEnableVertexAttribArray(2); // Texture
      glVertexAttribPointer(
          2, 2, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 2 * sizeof(float);

      glEnableVertexAttribArray(3); // BiTangent
      glVertexAttribPointer(
          3, 3, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 3 * sizeof(uint);

      glEnableVertexAttribArray(4); // Bones
      glVertexAttribPointer(
          4, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
      offset += 4 * sizeof(unsigned int);

      glEnableVertexAttribArray(5); // Weights
      glVertexAttribPointer(
          5, 4, GL_FLOAT, GL_FALSE, sizeof(SkinVertex), BUFFER_OFFSET(offset));
    }
  }

  void Renderer::SetTexture(ubyte slotIndx, uint textureId)
  {
    // Slots:
    // 0 - 5 : 2D textures
    // 6 - 7 : Cube map textures
    // 0 -> Color Texture
    // 2 & 3 -> Skinning information
    // 7 -> Irradiance Map
    // Note: These are defaults.
    //  You can override these slots in your linked shader program
    assert(slotIndx < m_rhiSettings::textureSlotCount &&
           "You exceed texture slot count");
    m_textureSlots[slotIndx] = textureId;
    glActiveTexture(GL_TEXTURE0 + slotIndx);

    // Slot id 6 - 7 are cubemaps
    if (slotIndx < 6)
    {
      glBindTexture(GL_TEXTURE_2D, m_textureSlots[slotIndx]);
    }
    else
    {
      glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureSlots[slotIndx]);
    }
  }

  void Renderer::SetShadowMapTexture(EntityType type,
                                     uint textureId,
                                     ProgramPtr program)
  {
    if (m_bindedShadowMapCount >= m_rhiSettings::maxShadows)
    {
      return;
    }

    /*
     * Texture Slots:
     * 8-11: Directional and spot light shadow maps
     * 12-15: Point light shadow maps
     */

    if (type == EntityType::Entity_PointLight)
    {
      if (m_pointLightShadowCount < m_rhiSettings::maxPointLightShadows)
      {
        int curr = m_pointLightShadowCount +
                   m_rhiSettings::maxDirAndSpotLightShadows +
                   m_rhiSettings::textureSlotCount;
        glUniform1i(
            glGetUniformLocation(program->m_handle,
                                 ("LightData.pointLightShadowMap[" +
                                  std::to_string(m_pointLightShadowCount) + "]")
                                     .c_str()),
            curr);
        glActiveTexture(GL_TEXTURE0 + curr);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        m_bindedShadowMapCount++;
        m_pointLightShadowCount++;
      }
    }
    else
    {
      if (m_dirAndSpotLightShadowCount <
          m_rhiSettings::maxDirAndSpotLightShadows)
      {
        int curr =
            m_dirAndSpotLightShadowCount + m_rhiSettings::textureSlotCount;
        glUniform1i(glGetUniformLocation(
                        program->m_handle,
                        ("LightData.dirAndSpotLightShadowMap[" +
                         std::to_string(m_dirAndSpotLightShadowCount) + "]")
                            .c_str()),
                    curr);
        glActiveTexture(GL_TEXTURE0 + curr);
        glBindTexture(GL_TEXTURE_2D, textureId);
        m_bindedShadowMapCount++;
        m_dirAndSpotLightShadowCount++;
      }
    }
  }

  void Renderer::ResetShadowMapBindings(ProgramPtr program)
  {
    m_bindedShadowMapCount       = 0;
    m_dirAndSpotLightShadowCount = 0;
    m_pointLightShadowCount      = 0;
  }

  CubeMapPtr Renderer::GenerateCubemapFrom2DTexture(TexturePtr texture,
                                                    uint width,
                                                    uint height,
                                                    float exposure)
  {
    const RenderTargetSettigs set = {0,
                                     false,
                                     GraphicTypes::TargetCubeMap,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::FormatRGB,
                                     GraphicTypes::FormatRGB,
                                     GraphicTypes::TypeUnsignedByte,
                                     Vec4(0.0f)};
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

    mat->m_diffuseTexture           = texture;
    mat->m_vertexShader             = vert;
    mat->m_fragmentShader           = frag;
    mat->GetRenderState()->cullMode = CullingType::TwoSided;
    mat->Init();

    m_utilFramebuffer->UnInit();
    m_utilFramebuffer->Init({width, height, 0, false, false});
    m_utilFramebuffer->ClearAttachments();

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

  CubeMapPtr Renderer::GenerateIrradianceCubemap(CubeMapPtr cubemap,
                                                 uint width,
                                                 uint height)
  {
    const RenderTargetSettigs set = {0,
                                     false,
                                     GraphicTypes::TargetCubeMap,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::UVClampToEdge,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::SampleLinear,
                                     GraphicTypes::FormatRGB,
                                     GraphicTypes::FormatRGB,
                                     GraphicTypes::TypeUnsignedByte,
                                     Vec4(0.0f)};
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
    m_utilFramebuffer->Init({width, height, 0, false, false});

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

} // namespace ToolKit
