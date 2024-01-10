/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Texture.h"

#include "DirectionComponent.h"
#include "FileManager.h"
#include "FullQuadPass.h"
#include "Logger.h"
#include "Material.h"
#include "RenderSystem.h"
#include "Shader.h"
#include "TKOpenGL.h"
#include "TKStats.h"
#include "ToolKit.h"

#include "DebugNew.h"

namespace ToolKit
{

  // Texture
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(Texture, Resource);

  Texture::Texture()
  {
    m_settings  = {GraphicTypes::Target2D,
                   GraphicTypes::UVRepeat,
                   GraphicTypes::UVRepeat,
                   GraphicTypes::UVRepeat,
                   GraphicTypes::SampleLinearMipmapLinear,
                   GraphicTypes::SampleLinear,
                   GraphicTypes::FormatSRGB8_A8,
                   GraphicTypes::FormatRGBA,
                   GraphicTypes::TypeUnsignedByte,
                   1,
                   true};

    m_textureId = 0;
  }

  Texture::Texture(const String& file) : Texture() { SetFile(file); }

  void Texture::NativeConstruct() { Super::NativeConstruct(); }

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

    if (m_settings.Type == GraphicTypes::TypeFloat)
    {
      if ((m_imagef = GetFileManager()->GetHdriFile(GetFile(), &m_width, &m_height, &m_bytePerPixel, 4)))
      {
        m_loaded = true;
      }
    }
    else
    {
      if ((m_image = GetFileManager()->GetImageFile(GetFile(), &m_width, &m_height, &m_bytePerPixel, 4)))
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

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    if (m_settings.Type != GraphicTypes::TypeFloat)
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   (GLint) m_settings.InternalFormat,
                   m_width,
                   m_height,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   m_image);

      AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat));
    }
    else
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   (GLint) m_settings.InternalFormat,
                   m_width,
                   m_height,
                   0,
                   GL_RGBA,
                   GL_FLOAT,
                   m_imagef);

      AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat));
    }

    if (m_settings.GenerateMipMap)
    {
      glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) m_settings.MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint) m_settings.MagFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint) m_settings.WarpS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint) m_settings.WarpT);

// android does not have support for this
#ifndef __ANDROID__
    if constexpr (GL_EXT_texture_filter_anisotropic)
    {
      float aniso = 0.0f;
      glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso);
    }
#endif

    if (flushClientSideArray)
    {
      Clear();
    }

    m_initiated = true;
  }

  void Texture::UnInit()
  {
    if (!m_initiated)
    {
      return;
    }

    if (m_settings.Target == GraphicTypes::Target2D)
    {
      RemoveVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat));
    }
    else if (m_settings.Target == GraphicTypes::Target2DArray)
    {
      RemoveVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat) * m_settings.Layers);
    }
    else if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      RemoveVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat) * 6);
    }
    else
    {
      assert(false);
    }

    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
    m_initiated = false;
  }

  const TextureSettings& Texture::Settings() { return m_settings; }

  void Texture::Settings(const TextureSettings& settings) { m_settings = settings; }

  void Texture::Clear()
  {
    free(m_image);
    free(m_imagef);
    m_image  = nullptr;
    m_imagef = nullptr;
    m_loaded = false;
  }

  // DepthTexture
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(DepthTexture, Texture);

  void DepthTexture::Load() {}

  void DepthTexture::Clear() { UnInit(); }

  void DepthTexture::Init(int width, int height, bool stencil)
  {
    if (m_initiated)
    {
      return;
    }
    m_initiated = true;
    m_width     = width;
    m_height    = height;
    m_stencil   = stencil;

    // Create a default depth, depth-stencil buffer
    glGenRenderbuffers(1, &m_textureId);
    glBindRenderbuffer(GL_RENDERBUFFER, m_textureId);
    GLenum component = stencil ? GL_DEPTH24_STENCIL8 : GL_DEPTH_COMPONENT24;
    glRenderbufferStorage(GL_RENDERBUFFER, component, m_width, m_height);

    uint64 internalFormatSize = stencil ? 4 : 3;
    AddVRAMUsageInBytes(m_width * m_height * internalFormatSize);
  }

  void DepthTexture::UnInit()
  {
    if (m_textureId == 0 || !m_initiated)
    {
      return;
    }
    glDeleteRenderbuffers(1, &m_textureId);

    uint64 internalFormatSize = m_stencil ? 4 : 3;
    RemoveVRAMUsageInBytes(m_width * m_height * internalFormatSize);

    m_textureId = 0;
    m_initiated = false;
  }

  // CubeMap
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(CubeMap, Texture);

  CubeMap::CubeMap() : Texture() {}

  CubeMap::CubeMap(const String& file) : Texture() { SetFile(file); }

  CubeMap::~CubeMap() {}

  void CubeMap::Consume(RenderTargetPtr cubeMapTarget)
  {
    TextureSettings textureSettings;
    textureSettings.GenerateMipMap = false;
    textureSettings.InternalFormat = cubeMapTarget->Settings().InternalFormat;
    textureSettings.MinFilter      = cubeMapTarget->Settings().MinFilter;
    textureSettings.Target         = GraphicTypes::TargetCubeMap;
    textureSettings.Type           = GraphicTypes::TypeFloat;

    m_textureId                    = cubeMapTarget->m_textureId;
    m_width                        = cubeMapTarget->m_width;
    m_height                       = cubeMapTarget->m_height;

    m_settings                     = textureSettings;
    m_initiated                    = true;

    cubeMapTarget->m_initiated     = false;
    cubeMapTarget->m_textureId     = 0;
    cubeMapTarget                  = nullptr;
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
      GetLogger()->Log("Inappropriate postfix. Looking for \"px.png\": " + fullPath);
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
      if ((m_images[i] = GetFileManager()->GetImageFile(name, &m_width, &m_height, &m_bytePerPixel, 0)))
      {
        GetLogger()->Log("Missing file: " + name);
        GetLogger()->Log("Cube map loading requires additional 5 png files with postfix "
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

    // This will be used when deleting the texture
    m_settings.InternalFormat = GraphicTypes::FormatRGBA;
    m_settings.Target         = GraphicTypes::TargetCubeMap;

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
      glTexImage2D(sides[i], 0, GL_RGBA, m_width, m_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_images[i]);
    }

    AddVRAMUsageInBytes(m_width * m_height * 4 * 6);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

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
      free(m_images[i]);
      m_images[i] = nullptr;
    }
    m_loaded = false;
  }

  // Hdri
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(Hdri, Texture);

  Hdri::Hdri()
  {
    m_settings.InternalFormat = GraphicTypes::FormatRGBA16F;
    m_settings.Type           = GraphicTypes::TypeFloat;
    m_settings.MinFilter      = GraphicTypes::SampleLinear;
    m_settings.GenerateMipMap = false;
    m_exposure                = 1.0f;
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
    Texture::Load();
  }

  void Hdri::Init(bool flushClientSideArray)
  {
    if (m_initiated || m_waitingForInit)
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
    m_initiated     = false;

    RenderTask task = {[this](Renderer* renderer) -> void
                       {
                         if (m_initiated)
                         {
                           m_waitingForInit = false;
                           return;
                         }

                         // Convert hdri image to cubemap images.
                         TexturePtr self = GetTextureManager()->Create<Texture>(GetFile());
                         uint size       = m_width / 4;
                         m_cubemap       = renderer->GenerateCubemapFrom2DTexture(self, size, 1.0f);

                         if (m_specularIBLTextureSize < 128)
                         {
                           m_specularIBLTextureSize = 128;
                         }

                         // Pre-filtered and mip mapped environment map
                         m_specularEnvMap = renderer->GenerateSpecularEnvMap(m_cubemap,
                                                                             m_specularIBLTextureSize,
                                                                             Renderer::RHIConstants::SpecularIBLLods);

                         // Generate diffuse irradience cubemap images
                         size             = m_width / 32;
                         m_diffuseEnvMap  = renderer->GenerateDiffuseEnvMap(m_cubemap, size);

                         m_initiated      = true;
                         m_waitingForInit = false;
                       }};

    GetRenderSystem()->AddRenderTask(task);
    m_waitingForInit = true;
  }

  void Hdri::UnInit()
  {
    if (m_initiated)
    {
      m_cubemap->UnInit();
      m_diffuseEnvMap->UnInit();
      m_specularEnvMap->UnInit();
    }

    m_waitingForInit = false;

    Texture::UnInit();
  }

  bool Hdri::IsTextureAssigned() { return !GetFile().empty(); }

  // RenderTarget
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(RenderTarget, Texture);

  RenderTarget::RenderTarget() { m_settings = {}; }

  RenderTarget::~RenderTarget() {}

  void RenderTarget::NativeConstruct(uint width, uint height, const TextureSettings& settings)
  {
    Super::NativeConstruct();

    m_width    = width;
    m_height   = height;
    m_settings = settings;
  }

  void RenderTarget::Load() {}

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

    // This will be used when deleting the texture
    m_settings.InternalFormat = m_settings.InternalFormat;
    m_settings.Target         = m_settings.Target;
    m_settings.Layers         = m_settings.Layers;

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

      AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat));
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

      AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat) * 6);
    }
    else if (m_settings.Target == GraphicTypes::Target2DArray)
    {
      glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, (int) m_settings.InternalFormat, m_width, m_height, m_settings.Layers);
      AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_settings.InternalFormat) * m_settings.Layers);
    }

    glTexParameteri((int) m_settings.Target, GL_TEXTURE_WRAP_S, (int) m_settings.WarpS);
    glTexParameteri((int) m_settings.Target, GL_TEXTURE_WRAP_T, (int) m_settings.WarpT);

    if (m_settings.Target == GraphicTypes::TargetCubeMap)
    {
      glTexParameteri((int) m_settings.Target, GL_TEXTURE_WRAP_R, (int) m_settings.WarpR);
    }

    glTexParameteri((int) m_settings.Target, GL_TEXTURE_MIN_FILTER, (int) m_settings.MinFilter);
    glTexParameteri((int) m_settings.Target, GL_TEXTURE_MAG_FILTER, (int) m_settings.MagFilter);

    m_initiated = true;
  }

  void RenderTarget::Reconstruct(uint width, uint height, const TextureSettings& settings)
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

  // TextureManager
  //////////////////////////////////////////////////////////////////////////

  TextureManager::TextureManager() { m_baseType = Texture::StaticClass(); }

  TextureManager::~TextureManager() {}

  bool TextureManager::CanStore(ClassMeta* Class)
  {
    if (Class->IsSublcassOf(Texture::StaticClass()))
    {
      return true;
    }

    return false;
  }

  String TextureManager::GetDefaultResource(ClassMeta* Class)
  {
    if (Class == Hdri::StaticClass())
    {
      return TexturePath("defaultHDRI.hdr", true);
    }
    else
    {
      return TexturePath("default.png", true);
    }
  }
} // namespace ToolKit
