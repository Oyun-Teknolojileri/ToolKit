#include "Texture.h"

#include "DirectionComponent.h"
#include "GL/glew.h"
#include "ToolKit.h"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{

  Texture::Texture(const TextureSettings& settings)
  {
    m_textureSettings = settings;
    m_textureId       = 0;
  }

  Texture::Texture(String file, const TextureSettings& settings)
      : Texture(settings)
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
    Clear();
  }

  void Texture::Load()
  {
    if (m_loaded)
    {
      return;
    }

    if (m_textureSettings.Type == GraphicTypes::TypeFloat)
    {
      if ((m_imagef =
               GetFileManager()
                   ->GetHdriFile(GetFile(), &m_width, &m_height, &m_bytePP, 3)))
      {
        m_loaded = true;
      }
    }
    else
    {
      if ((m_image = GetFileManager()->GetImageFile(GetFile(),
                                                    &m_width,
                                                    &m_height,
                                                    &m_bytePP,
                                                    4)))
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

    // Sanity checks
    if ((m_image == nullptr && m_imagef == nullptr))
    {
      assert(0 && "No texture data.");
      return;
    }

    if (m_width <= 0 || m_height <= 0)
    {
      assert(0 && "Zero texture size.");
      return;
    }

    GLint currId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_textureSettings.Type != GraphicTypes::TypeFloat)
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   (GLint) m_textureSettings.InternalFormat,
                   m_width,
                   m_height,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   m_image);
    }
    else
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   (GLint) m_textureSettings.InternalFormat,
                   m_width,
                   m_height,
                   0,
                   GL_RGB,
                   GL_FLOAT,
                   m_imagef);
    }

    if (m_textureSettings.GenerateMipMap)
    {
      glGenerateMipmap(GL_TEXTURE_2D);

      glTexParameteri(GL_TEXTURE_2D,
                      GL_TEXTURE_MIN_FILTER,
                      (GLint) m_textureSettings.MipMapMinFilter);
    }

    glTexParameteri(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    (GLint) m_textureSettings.MinFilter);

#ifndef TK_GL_ES_3_0
    if (GL_EXT_texture_filter_anisotropic)
    {
      float aniso = 0.0f;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }
#endif // TK_GL_ES_3_0

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
    m_textureId = 0;
    m_initiated = false;
  }

  const TextureSettings& Texture::GetTextureSettings()
  {
    return m_textureSettings;
  }

  void Texture::SetTextureSettings(const TextureSettings& settings)
  {
    m_textureSettings = settings;
  }

  void Texture::Clear()
  {
    stbi_image_free(m_image);
    stbi_image_free(m_imagef);
    m_image  = nullptr;
    m_imagef = nullptr;
    m_loaded = false;
  }

  CubeMap::CubeMap() : Texture() {}

  CubeMap::CubeMap(String file) : Texture() { SetFile(file); }

  CubeMap::CubeMap(uint cubemapId)
  {
    m_textureId = cubemapId;
    m_initiated = true;
  }

  CubeMap::~CubeMap() { UnInit(); }

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
      if ((m_images[i] =
               GetFileManager()
                   ->GetImageFile(name, &m_width, &m_height, &m_bytePP, 0)))
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

    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP,
                    GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);

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
    m_textureSettings.InternalFormat  = GraphicTypes::FormatRGB32F;
    m_textureSettings.Type            = GraphicTypes::TypeFloat;
    m_textureSettings.MinFilter       = GraphicTypes::SampleLinear;
    m_textureSettings.MipMapMinFilter = GraphicTypes::SampleLinearMipmapLinear;
    m_textureSettings.GenerateMipMap  = false;
    m_exposure                        = 1.0f;

    m_texToCubemapMat                 = std::make_shared<Material>();
    m_cubemapToIrradiancemapMat       = std::make_shared<Material>();
    m_irradianceCubemap               = std::make_shared<CubeMap>(0);
    m_equirectangularTexture = std::make_shared<Texture>(static_cast<uint>(0));
  }

  Hdri::Hdri(const String& file) : Hdri() { SetFile(file); }

  Hdri::~Hdri() { UnInit(); }

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

    // Init 2D hdri texture
    Texture::Init(flushClientSideArray);

    // Only ready after cubemap constructed and irradiance calculated.
    m_initiated     = false;

    RenderTask task = {
        [this, flushClientSideArray](Renderer* renderer) -> void
        {
          // Convert hdri image to cubemap images.
          m_cubemap = renderer->GenerateCubemapFrom2DTexture(
              GetTextureManager()->Create<Texture>(GetFile()),
              m_width,
              m_width,
              1.0f);

          // Generate mip maps of cubemap
          glBindTexture(GL_TEXTURE_CUBE_MAP, m_cubemap->m_textureId);
          glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

          // TODO since there is a "update ibl" button, we can make these
          // parameter variant
          const int prefilteredEnvMapSize =
              512; // TODO member variable or parameter variant
          // Pre-filtered and mip mapped environment map
          m_prefilteredEnvMap = renderer->GenerateEnvPrefilteredMap(
              m_cubemap,
              prefilteredEnvMapSize,
              prefilteredEnvMapSize,
              Renderer::RHIConstants::specularIBLLods);

          // Pre-compute BRDF lut
          if (!m_quadPass)
          {
            m_quadPass = std::make_shared<FullQuadPass>();
          }
          const int lutSize = 512; // TODO member variable or parameter variant
          if (!m_brdfLut)
          {
            RenderTargetSettigs set;
            set.InternalFormat = GraphicTypes::FormatRG16F;
            set.Format         = GraphicTypes::FormatRG;
            m_brdfLut = std::make_shared<RenderTarget>(lutSize, lutSize, set);
            m_brdfLut->Init();
          }
          if (!m_utilFramebuffer)
          {
            m_utilFramebuffer = std::make_shared<Framebuffer>();
            m_utilFramebuffer->Init({lutSize, lutSize, false, false});
            m_utilFramebuffer->SetAttachment(
                Framebuffer::Attachment::ColorAttachment0,
                m_brdfLut);
          }

          m_quadPass->m_params.FrameBuffer = m_utilFramebuffer;
          m_quadPass->m_params.FragmentShader =
              GetShaderManager()->Create<Shader>(
                  ShaderPath("BRDFLutFrag.shader", true));

          m_quadPass->SetRenderer(renderer);
          m_quadPass->PreRender();
          m_quadPass->Render();
          m_quadPass->PostRender();

          // Generate irradience cubemap images
          m_irradianceCubemap =
              renderer->GenerateEnvIrradianceMap(m_cubemap,
                                                 m_width / 64,
                                                 m_width / 64);

          m_initiated = true;
        }};

    GetRenderSystem()->AddRenderTask(task);
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

  bool Hdri::IsTextureAssigned() { return !GetFile().empty(); }

  RenderTarget::RenderTarget() : Texture() {}

  RenderTarget::RenderTarget(uint width,
                             uint height,
                             const RenderTargetSettigs& settings)
      : RenderTarget()
  {
    m_width    = width;
    m_height   = height;
    m_settings = settings;
  }

  void RenderTarget::Load() {}

  RenderTarget::RenderTarget(Texture* texture)
  {
    m_width     = texture->m_width;
    m_height    = texture->m_height;
    m_textureId = texture->m_textureId;
    m_initiated = true;
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
    else if (m_settings.Target == GraphicTypes::Target2DArray)
    {
      glGetIntegerv(GL_TEXTURE_BINDING_2D_ARRAY, &currId);
    }

    // Create frame buffer color texture
    glGenTextures(1, &m_textureId);
    glBindTexture((int) m_settings.Target, m_textureId);

    if (m_settings.Target == GraphicTypes::Target2D)
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   (int) m_settings.InternalFormat,
                   m_width,
                   m_height,
                   0,
                   (int) m_settings.Format,
                   (int) m_settings.Type,
                   0);
    }
    else if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      for (unsigned int i = 0; i < 6; ++i)
      {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0,
                     (int) m_settings.InternalFormat,
                     m_width,
                     m_height,
                     0,
                     (int) m_settings.Format,
                     (int) m_settings.Type,
                     0);
      }
    }
    else if (m_settings.Target == GraphicTypes::Target2DArray)
    {
      glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                     1,
                     (int) m_settings.InternalFormat,
                     m_width,
                     m_height,
                     m_settings.Layers);
    }

    glTexParameteri((int) m_settings.Target,
                    GL_TEXTURE_WRAP_S,
                    (int) m_settings.WarpS);

    glTexParameteri((int) m_settings.Target,
                    GL_TEXTURE_WRAP_T,
                    (int) m_settings.WarpT);

    if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      glTexParameteri((int) m_settings.Target,
                      GL_TEXTURE_WRAP_R,
                      (int) m_settings.WarpR);
    }
    glTexParameteri((int) m_settings.Target,
                    GL_TEXTURE_MIN_FILTER,
                    (int) m_settings.MinFilter);

    glTexParameteri((int) m_settings.Target,
                    GL_TEXTURE_MAG_FILTER,
                    (int) m_settings.MagFilter);

    m_initiated = true;

    // Restore previous render target.
    glBindTexture((int) m_settings.Target, currId);
  }

  void RenderTarget::Reconstruct(uint width,
                                 uint height,
                                 const RenderTargetSettigs& settings)
  {
    UnInit();
    m_width    = width;
    m_height   = height;
    m_settings = settings;
    Init();
  }

  void RenderTarget::ReconstructIfNeeded(uint width, uint height)
  {
    if (!m_initiated || m_width != width || m_height != height)
    {
      Reconstruct(width, height, m_settings);
    }
  }

  const RenderTargetSettigs& RenderTarget::GetSettings() const
  {
    return m_settings;
  }

  TextureManager::TextureManager() { m_type = ResourceType::Texture; }

  TextureManager::~TextureManager() {}

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
