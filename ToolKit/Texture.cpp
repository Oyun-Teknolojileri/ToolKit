#include "Texture.h"

#include <memory>

#include "ToolKit.h"
#include "GL/glew.h"
#include "DebugNew.h"
#include "DirectionComponent.h"

namespace ToolKit
{

  Texture::Texture(bool floatFormat)
  {
    m_floatFormat = floatFormat;
    m_textureId   = 0;
  }

  Texture::Texture(String file, bool floatFormat) : Texture(floatFormat)
  {
    SetFile(file);
  }

  Texture::Texture(uint textureId)
  {
    m_textureId = textureId;
    m_initiated = true;
  }

  Texture::~Texture()
  {
    UnInit();
  }

  void Texture::Load()
  {
    if (m_loaded)
    {
      return;
    }

    if (m_floatFormat)
    {
      if ((m_imagef = GetFileManager()->GetHdriFile(
               GetFile().c_str(), &m_width, &m_height, &m_bytePP, 3)))
      {
        m_loaded = true;
      }
    }
    else
    {
      if ((m_image = GetFileManager()->GetImageFile(
               GetFile(), &m_width, &m_height, &m_bytePP, 4)))
      {
        m_loaded = true;
      }
    }
  }

  void Texture::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    // Sanity check
    if ((m_image == nullptr && m_imagef == nullptr) || m_width <= 0 ||
        m_height <= 0)
    {
      return;
    }

    GLint currId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_floatFormat)
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGB16F,
                   m_width,
                   m_height,
                   0,
                   GL_RGB,
                   GL_FLOAT,
                   m_imagef);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGBA,
                   m_width,
                   m_height,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   m_image);
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(
          GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (GL_EXT_texture_filter_anisotropic)
    {
      float aniso = 0.0f;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }

    if (flushClientSideArray)
    {
      Clear();
    }

    glBindTexture(GL_TEXTURE_2D, currId);
    m_initiated = true;
  }

  void Texture::UnInit()
  {
    glDeleteTextures(1, &m_textureId);
    Clear();
    m_initiated = false;
  }

  void Texture::Clear()
  {
    stbi_image_free(m_image);
    stbi_image_free(m_imagef);
    m_image  = nullptr;
    m_imagef = nullptr;
    m_loaded = false;
  }

  CubeMap::CubeMap() : Texture()
  {
  }

  CubeMap::CubeMap(String file) : Texture()
  {
    SetFile(file);
  }

  CubeMap::CubeMap(uint cubemapId)
  {
    m_textureId = cubemapId;
    m_initiated = true;
  }

  CubeMap::~CubeMap()
  {
    UnInit();
  }

  void CubeMap::Load()
  {
    if (m_loaded)
    {
      return;
    }

    m_images.resize(6);
    String fullPath = GetFile();
    size_t pos      = fullPath.find("px.png");
    if (pos == String::npos)
    {
      GetLogger()->Log("Inappropriate postfix. Looking for \"px.png\": " +
                       fullPath);
      return;
    }

    String file = fullPath.substr(0, pos);

    for (int i = 0; i < 6; i++)
    {
      String postfix = "px.png";
      switch (i)
      {
      case 1:
        postfix = "nx.png";
        break;
      case 2:
        postfix = "py.png";
        break;
      case 3:
        postfix = "ny.png";
        break;
      case 4:
        postfix = "pz.png";
        break;
      case 5:
        postfix = "nz.png";
        break;
      }

      String name = file + postfix;
      if ((m_images[i] = GetFileManager()->GetImageFile(
               name, &m_width, &m_height, &m_bytePP, 0)))
      {
        GetLogger()->Log("Missing file: " + name);
        GetLogger()->Log(
            "Cube map loading requires additional 5 png files with postfix "
            "\"nx py ny pz nz\".");
        m_loaded = false;

        Clear();
        return;
      }
    }

    m_loaded = true;
  }

  void CubeMap::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    if (!m_loaded)
    {
      return;
    }

    // Sanity check.
    if (m_images.size() != size_t(6) || m_width <= 0 || m_height <= 0)
    {
      return;
    }

    GLint currId;
    glGetIntegerv(GL_TEXTURE_CUBE_MAP, &currId);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureId);

    uint sides[6] = {GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                     GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

    for (int i = 0; i < 6; i++)
    {
      glTexImage2D(sides[i],
                   0,
                   GL_RGBA,
                   m_width,
                   m_width,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   m_images[i]);
    }

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, currId);

    if (flushClientSideArray)
    {
      Clear();
    }

    m_initiated = true;
  }

  void CubeMap::UnInit()
  {
    Texture::UnInit();
    Clear();
    m_initiated = false;
  }

  void CubeMap::Clear()
  {
    for (int i = 0; i < m_images.size(); i++)
    {
      stbi_image_free(m_images[i]);
      m_images[i] = nullptr;
    }
    m_loaded = false;
  }

  Hdri::Hdri()
  {
    m_floatFormat = true;
    m_exposure    = 1.0f;

    m_texToCubemapMat           = std::make_shared<Material>();
    m_cubemapToIrradiancemapMat = std::make_shared<Material>();
    m_irradianceCubemap         = std::make_shared<CubeMap>(0);
    m_cubemap                   = std::make_shared<CubeMap>(0);
    m_equirectangularTexture = std::make_shared<Texture>(static_cast<uint>(0));
  }

  Hdri::Hdri(const String& file) : Hdri()
  {
    SetFile(file);
  }

  Hdri::~Hdri()
  {
    UnInit();
  }

  void Hdri::Load()
  {
    if (m_loaded)
    {
      return;
    }

    // Load hdri image
    stbi_set_flip_vertically_on_load(true);
    Texture::Load();
    stbi_set_flip_vertically_on_load(false);
  }

  void Hdri::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    // Sanity check.
    if (m_imagef == nullptr || m_width <= 0 || m_height <= 0)
    {
      return;
    }

    CreateFramebuffersForCubeMaps();

    // Init 2D hdri texture
    Texture::Init();

    // Convert hdri image to cubemap images
    GenerateCubemapFrom2DTexture();

    // Generate irradience cubemap images
    GenerateIrradianceMap();

    DeleteFramebuffers();
  }

  void Hdri::UnInit()
  {
    if (m_initiated)
    {
      m_cubemap->UnInit();
      m_irradianceCubemap->UnInit();
    }
    Texture::UnInit();
  }

  bool Hdri::IsTextureAssigned()
  {
    return (GetFile().size() != 0);
  }

  uint Hdri::GetCubemapId()
  {
    return m_cubemap->m_textureId;
  }

  void Hdri::SetCubemapId(uint id)
  {
    m_cubemap->m_textureId = id;
  }

  uint Hdri::GetIrradianceCubemapId()
  {
    if (m_irradianceCubemap)
    {
      return m_irradianceCubemap->m_textureId;
    }
    else
    {
      return 0;
    }
  }

  void Hdri::SetIrradianceCubemapId(uint id)
  {
    m_irradianceCubemap->m_textureId = id;
  }

  uint Hdri::GenerateCubemapBuffers(struct CubeMapSettings cubeMapSettings)
  {
    GLint currId;
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &currId);

    // Create buffers for cubemap textures
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    for (unsigned int i = 0; i < 6; ++i)
    {
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                   cubeMapSettings.level,
                   cubeMapSettings.internalformat,
                   cubeMapSettings.width,
                   cubeMapSettings.height,
                   0,
                   cubeMapSettings.format,
                   cubeMapSettings.type,
                   cubeMapSettings.pixels);
    }

    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, cubeMapSettings.wrapSet);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, cubeMapSettings.wrapSet);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, cubeMapSettings.wrapSet);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, cubeMapSettings.filterSet);
    glTexParameteri(
        GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, cubeMapSettings.filterSet);

    glBindTexture(GL_TEXTURE_CUBE_MAP, currId);

    return textureId;
  }

  void Hdri::RenderToCubeMap(GLuint fbo,
                             const Mat4 views[6],
                             CameraPtr cam,
                             uint cubeMapTextureId,
                             int width,
                             int height,
                             MaterialPtr mat)
  {
    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    // Render cube from 6 different angles for 6 images of cubemap
    for (int i = 0; i < 6; ++i)
    {
      Vec3 pos;
      Quaternion rot;
      Vec3 sca;
      DecomposeMatrix(views[i], &pos, &rot, &sca);

      cam->m_node->SetTranslation(ZERO, TransformationSpace::TS_WORLD);
      cam->m_node->SetOrientation(rot, TransformationSpace::TS_WORLD);
      cam->m_node->SetScale(sca);

      glFramebufferTexture2D(GL_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             cubeMapTextureId,
                             0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      GetRenderer()->SetViewportSize(width, height);

      CullingType currType            = mat->GetRenderState()->cullMode;
      mat->GetRenderState()->cullMode = CullingType::TwoSided;
      GetRenderer()->DrawCube(cam.get(), mat);
      mat->GetRenderState()->cullMode = currType;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  void Hdri::CreateFramebuffersForCubeMaps()
  {
    auto createFbFn = [](GLuint& fbo, GLuint& rbo, int width, int height) {
      glGenFramebuffers(1, &fbo);
      glGenRenderbuffers(1, &rbo);

      glBindFramebuffer(GL_FRAMEBUFFER, fbo);
      glBindRenderbuffer(GL_RENDERBUFFER, rbo);
      glRenderbufferStorage(
          GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
      glFramebufferRenderbuffer(
          GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

      // Check if framebuffer is complete
      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      {
        GetLogger()->Log("Error while creating framebuffer.");
      }
    };

    GLint lastFBO;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &lastFBO);

    createFbFn(m_fbo, m_rbo, m_width, m_width);
    createFbFn(m_irradianceFbo, m_irradianceRbo, m_width / 64, m_width / 64);

    glBindFramebuffer(GL_FRAMEBUFFER, lastFBO);
  }

  void Hdri::DeleteFramebuffers()
  {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteRenderbuffers(1, &m_rbo);
    glDeleteFramebuffers(1, &m_irradianceFbo);
    glDeleteRenderbuffers(1, &m_irradianceRbo);
  }

  void Hdri::GenerateCubemapFrom2DTexture()
  {
    GLuint cubemapTextureId = GenerateCubemapBuffers({0,
                                                      GL_RGB,
                                                      m_width,
                                                      m_width,
                                                      GL_RGB,
                                                      GL_UNSIGNED_BYTE,
                                                      nullptr,
                                                      GL_CLAMP_TO_EDGE,
                                                      GL_LINEAR});

    // Views for 6 different angles
    CameraPtr cam = std::make_shared<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    // Create material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("equirectToCubeVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("equirectToCubeFrag.shader", true));
    frag->m_shaderParams["Exposure"] = m_exposure;

    m_equirectangularTexture->m_textureId = m_textureId;
    m_texToCubemapMat->m_diffuseTexture   = m_equirectangularTexture;
    m_texToCubemapMat->m_vertexShader     = vert;
    m_texToCubemapMat->m_fragmetShader    = frag;
    m_texToCubemapMat->Init();

    RenderToCubeMap(m_fbo,
                    views,
                    cam,
                    cubemapTextureId,
                    m_width,
                    m_width,
                    m_texToCubemapMat);

    m_cubemap->m_textureId = cubemapTextureId;
  }

  void Hdri::GenerateIrradianceMap()
  {
    GLuint irradianceTextureId = GenerateCubemapBuffers({0,
                                                         GL_RGB,
                                                         m_width / 64,
                                                         m_width / 64,
                                                         GL_RGB,
                                                         GL_UNSIGNED_BYTE,
                                                         nullptr,
                                                         GL_CLAMP_TO_EDGE,
                                                         GL_LINEAR});

    // Views for 6 different angles
    CameraPtr cam = std::make_shared<Camera>();
    cam->SetLens(glm::radians(90.0f), 1.0f, 1.0f, 0.1f, 10.0f);
    Mat4 views[] = {
        glm::lookAt(ZERO, Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, -1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(ZERO, Vec3(0.0f, 0.0f, -1.0f), Vec3(0.0f, -1.0f, 0.0f))};

    // Create material
    ShaderPtr vert = GetShaderManager()->Create<Shader>(
        ShaderPath("irradianceGenerateVert.shader", true));
    ShaderPtr frag = GetShaderManager()->Create<Shader>(
        ShaderPath("irradianceGenerateFrag.shader", true));

    m_cubemapToIrradiancemapMat->m_cubeMap       = m_cubemap;
    m_cubemapToIrradiancemapMat->m_vertexShader  = vert;
    m_cubemapToIrradiancemapMat->m_fragmetShader = frag;
    m_cubemapToIrradiancemapMat->Init();

    RenderToCubeMap(m_irradianceFbo,
                    views,
                    cam,
                    irradianceTextureId,
                    m_width / 64,
                    m_width / 64,
                    m_cubemapToIrradiancemapMat);

    m_irradianceCubemap->m_textureId = irradianceTextureId;
  }

  RenderTarget::RenderTarget() : Texture()
  {
  }

  RenderTarget::RenderTarget(uint width,
                             uint height,
                             const RenderTargetSettigs& settings)
      : RenderTarget()
  {
    m_width         = width;
    m_height        = height;
    m_frameBufferId = 0;
    m_depthBufferId = 0;
    m_settings      = settings;
  }

  RenderTarget::~RenderTarget()
  {
    UnInit();
  }

  void RenderTarget::Load()
  {
  }

  void RenderTarget::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    if (m_width <= 0 || m_height <= 0)
    {
      return;
    }

    GLint currId = 0; // Don't override the current render target.
    if (m_settings.Target == GraphicTypes::Target2D)
    {
      glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);
    }
    else if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &currId);
    }

    // Create frame buffer color texture
    glGenTextures(1, &m_textureId);
    glBindTexture(static_cast<int>(m_settings.Target), m_textureId);

    glTexParameteri(static_cast<int>(m_settings.Target),
                    GL_TEXTURE_WRAP_S,
                    static_cast<int>(m_settings.WarpS));
    glTexParameteri(static_cast<int>(m_settings.Target),
                    GL_TEXTURE_WRAP_T,
                    static_cast<int>(m_settings.WarpT));
    if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      glTexParameteri(static_cast<int>(m_settings.Target),
                      GL_TEXTURE_WRAP_R,
                      static_cast<int>(m_settings.WarpR));
    }
    glTexParameteri(static_cast<int>(m_settings.Target),
                    GL_TEXTURE_MIN_FILTER,
                    static_cast<int>(m_settings.MinFilter));
    glTexParameteri(static_cast<int>(m_settings.Target),
                    GL_TEXTURE_MAG_FILTER,
                    static_cast<int>(m_settings.MagFilter));

    if (m_settings.Target == GraphicTypes::Target2D)
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   static_cast<int>(m_settings.InternalFormat),
                   m_width,
                   m_height,
                   0,
                   static_cast<int>(m_settings.Format),
                   static_cast<int>(m_settings.Type),
                   0);
    }
    else if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      for (unsigned int i = 0; i < 6; ++i)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     static_cast<int>(m_settings.InternalFormat),
                     m_width,
                     m_height,
                     0,
                     static_cast<int>(m_settings.Format),
                     static_cast<int>(m_settings.Type),
                     0);
      }
    }

    if (m_settings.useBorderColor)
    {
      glTexParameterfv(static_cast<int>(m_settings.Target),
                       GL_TEXTURE_BORDER_COLOR,
                       &(m_settings.borderColor[0]));
    }

    glGenFramebuffers(1, &m_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

    // Attach 2D texture to this FBO
#ifndef __EMSCRIPTEN__
    bool msaaRTunsupported = false;
    if (glFramebufferTexture2DMultisampleEXT == nullptr)
    {
      msaaRTunsupported = true;

      static bool notReported = true;
      if (notReported)
      {
        GetLogger()->Log(
            "Unsupported Extension: glFramebufferTexture2DMultisampleEXT");
        notReported = false;
      }
    }

    bool goForMsaa = m_settings.Msaa > 0 && !msaaRTunsupported;
    if (goForMsaa)
    {
      if (m_settings.Target == GraphicTypes::Target2D)
      {
        glFramebufferTexture2DMultisampleEXT(
            GL_FRAMEBUFFER,
            static_cast<int>(m_settings.Attachment),
            static_cast<int>(m_settings.Target),
            m_textureId,
            0,
            m_settings.Msaa);
      }
      else if (m_settings.Target == GraphicTypes::TargetCubeMap)
      {
        for (unsigned int i = 0; i < 6; ++i)
        {
          glFramebufferTexture2DMultisampleEXT(
              GL_FRAMEBUFFER,
              static_cast<int>(m_settings.Attachment),
              GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
              m_textureId,
              0,
              m_settings.Msaa);
        }
      }
    }
    else
#endif
    {
      glFramebufferTexture(GL_FRAMEBUFFER,
                           static_cast<int>(m_settings.Attachment),
                           m_textureId,
                           0);
    }

    if (m_settings.DepthStencil)
    {
      glGenRenderbuffers(1, &m_depthBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferId);

#ifndef __EMSCRIPTEN__
      if (goForMsaa)
      {
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
                                            m_settings.Msaa,
                                            GL_DEPTH24_STENCIL8,
                                            m_width,
                                            m_height);
      }
      else
#endif
      {
        glRenderbufferStorage(
            GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
      }

      // Attach depth buffer to FBO
      glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                GL_DEPTH_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER,
                                m_depthBufferId);
    }

    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) !=
        GL_FRAMEBUFFER_COMPLETE)
    {
      m_initiated = false;
      glDeleteTextures(1, &m_textureId);
      glDeleteFramebuffers(1, &m_frameBufferId);
      glDeleteRenderbuffers(1, &m_depthBufferId);
    }
    else
    {
      m_initiated = true;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Restore backbuffer.

    // Restore previous render target.
    glBindTexture(static_cast<int>(m_settings.Target), currId);
  }

  void RenderTarget::UnInit()
  {
    Texture::UnInit();

    glDeleteFramebuffers(1, &m_frameBufferId);
    glDeleteRenderbuffers(1, &m_depthBufferId);
    m_initiated = false;
  }

  void RenderTarget::Reconstrcut(uint width,
                                 uint height,
                                 const RenderTargetSettigs& settings)
  {
    UnInit();
    m_width    = width;
    m_height   = height;
    m_settings = settings;
    Init();
  }

  const RenderTargetSettigs& RenderTarget::GetSettings() const
  {
    return m_settings;
  }

  TextureManager::TextureManager()
  {
    m_type = ResourceType::Texture;
  }

  TextureManager::~TextureManager()
  {
  }

  bool TextureManager::CanStore(ResourceType t)
  {
    if (t == ResourceType::Texture || t == ResourceType::CubeMap ||
        t == ResourceType::RenderTarget || t == ResourceType::Hdri)
    {
      return true;
    }

    return false;
  }

  ResourcePtr TextureManager::CreateLocal(ResourceType type)
  {
    Texture* tex = nullptr;
    switch (type)
    {
    case ResourceType::Texture:
      tex = new Texture();
      break;
    case ResourceType::CubeMap:
      tex = new CubeMap();
      break;
    case ResourceType::RenderTarget:
      tex = new RenderTarget();
      break;
    case ResourceType::Hdri:
      tex = new Hdri();
      break;
    default:
      assert(false);
      break;
    }
    return ResourcePtr(tex);
  }

  String TextureManager::GetDefaultResource(ResourceType type)
  {
    if (type == ResourceType::Hdri)
    {
      return TexturePath("defaultHDRI.hdr", true);
    }
    else
    {
      return TexturePath("default.png", true);
    }
  }

} // namespace ToolKit
