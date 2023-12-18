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

  Texture::Texture(const TextureSettings& settings)
  {
    m_textureSettings = settings;
    m_textureId       = 0;
  }

  Texture::Texture(const String& file, const TextureSettings& settings) : Texture(settings) { SetFile(file); }

  void Texture::NativeConstruct(uint textureId)
  {
    Super::NativeConstruct();

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
      if ((m_imagef = GetFileManager()->GetHdriFile(GetFile(), &m_width, &m_height, &m_bytePP, 4)))
      {
        m_loaded = true;
      }
    }
    else
    {
      if ((m_image = GetFileManager()->GetImageFile(GetFile(), &m_width, &m_height, &m_bytePP, 4)))
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

      TKStats::AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat));
    }
    else
    {
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   (GLint) m_textureSettings.InternalFormat,
                   m_width,
                   m_height,
                   0,
                   GL_RGBA,
                   GL_FLOAT,
                   m_imagef);

      TKStats::AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat));
    }

    if (m_textureSettings.GenerateMipMap)
    {
      glGenerateMipmap(GL_TEXTURE_2D);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) m_textureSettings.MipMapMinFilter);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint) m_textureSettings.MinFilter);

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

    glBindTexture(GL_TEXTURE_2D, currId);
    m_initiated = true;
  }

  void Texture::UnInit()
  {
    if (!m_initiated)
    {
      return;
    }

    if (m_textureSettings.Target == GraphicTypes::Target2D)
    {
      TKStats::RemoveVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat));
    }
    else if (m_textureSettings.Target == GraphicTypes::Target2DArray)
    {
      TKStats::RemoveVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat) *
                                      m_textureSettings.Layers);
    }
    else if (m_textureSettings.Target == GraphicTypes::TargetCubeMap)
    {
      TKStats::RemoveVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat) * 6);
    }
    else
    {
      assert(false);
    }

    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
    m_initiated = false;
  }

  const TextureSettings& Texture::GetTextureSettings() { return m_textureSettings; }

  void Texture::SetTextureSettings(const TextureSettings& settings) { m_textureSettings = settings; }

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

    TKStats::AddVRAMUsageInBytes(m_width * m_height * 4);
  }

  void DepthTexture::UnInit()
  {
    if (m_textureId == 0 || !m_initiated)
    {
      return;
    }
    glDeleteRenderbuffers(1, &m_textureId);

    TKStats::RemoveVRAMUsageInBytes(m_width * m_height * 4);

    m_textureId = 0;
    m_initiated = false;
  }

  // CubeMap
  //////////////////////////////////////////////////////////////////////////

  TKDefineClass(CubeMap, Texture);

  CubeMap::CubeMap() : Texture() {}

  CubeMap::CubeMap(const String& file) : Texture() { SetFile(file); }

  CubeMap::~CubeMap() {}

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
      if ((m_images[i] = GetFileManager()->GetImageFile(name, &m_width, &m_height, &m_bytePP, 0)))
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
    m_textureSettings.InternalFormat = GraphicTypes::FormatRGBA;
    m_textureSettings.Target         = GraphicTypes::TargetCubeMap;

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
      glTexImage2D(sides[i], 0, GL_RGBA, m_width, m_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_images[i]);
    }

    TKStats::AddVRAMUsageInBytes(m_width * m_height * 4 * 6);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

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
    m_textureSettings.InternalFormat  = GraphicTypes::FormatRGBA16F;
    m_textureSettings.Type            = GraphicTypes::TypeFloat;
    m_textureSettings.MinFilter       = GraphicTypes::SampleLinear;
    m_textureSettings.MipMapMinFilter = GraphicTypes::SampleLinearMipmapLinear;
    m_textureSettings.GenerateMipMap  = false;
    m_exposure                        = 1.0f;

    m_texToCubemapMat                 = MakeNewPtr<Material>();
    m_cubemapToDiffuseEnvMapMat       = MakeNewPtr<Material>();
    m_diffuseEnvMap                   = MakeNewPtr<CubeMap>();
    m_equirectangularTexture          = MakeNewPtr<Texture>();
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

    RenderTask task = {[this, flushClientSideArray](Renderer* renderer) -> void
                       {
                         if (m_initiated)
                         {
                           m_waitingForInit = false;
                           return;
                         }

                         // Convert hdri image to cubemap images.
                         m_cubemap =
                             renderer->GenerateCubemapFrom2DTexture(GetTextureManager()->Create<Texture>(GetFile()),
                                                                    m_width / 4,
                                                                    m_width / 4,
                                                                    1.0f);

                         const int specularEnvMapSize = m_specularIBLTextureSize;
                         // Pre-filtered and mip mapped environment map
                         m_specularEnvMap             = renderer->GenerateSpecularEnvMap(m_cubemap,
                                                                             specularEnvMapSize,
                                                                             specularEnvMapSize,
                                                                             Renderer::RHIConstants::SpecularIBLLods);

                         // Generate diffuse irradience cubemap images
                         m_diffuseEnvMap  = renderer->GenerateDiffuseEnvMap(m_cubemap, m_width / 32, m_width / 32);

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

  RenderTarget::RenderTarget() : Texture() {}

  RenderTarget::~RenderTarget()
  {
    m_textureSettings.GenerateMipMap = false;
    m_textureSettings.InternalFormat = m_settings.InternalFormat;
    m_textureSettings.Layers         = m_settings.Layers;
    m_textureSettings.MinFilter      = m_settings.MinFilter;
    m_textureSettings.Target         = m_settings.Target;
    m_textureSettings.Type           = m_settings.Type;
  }

  void RenderTarget::NativeConstruct(uint width, uint height, const RenderTargetSettigs& settings)
  {
    Super::NativeConstruct();

    m_width    = width;
    m_height   = height;
    m_settings = settings;
  }

  void RenderTarget::NativeConstruct(Texture* texture)
  {
    Super::NativeConstruct();

    m_width     = texture->m_width;
    m_height    = texture->m_height;
    m_textureId = texture->m_textureId;
    m_initiated = true;
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
    m_textureSettings.InternalFormat = m_settings.InternalFormat;
    m_textureSettings.Target         = m_settings.Target;
    m_textureSettings.Layers         = m_settings.Layers;

    GLint currId                     = 0; // Don't override the current render target.
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

      TKStats::AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat));
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

      TKStats::AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat) * 6);
    }
    else if (m_settings.Target == GraphicTypes::Target2DArray)
    {
      glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, (int) m_settings.InternalFormat, m_width, m_height, m_settings.Layers);

      TKStats::AddVRAMUsageInBytes(m_width * m_height * BytesOfFormat(m_textureSettings.InternalFormat) *
                                   m_settings.Layers);
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

    // Restore previous render target.
    glBindTexture((int) m_settings.Target, currId);
  }

  void RenderTarget::Reconstruct(uint width, uint height, const RenderTargetSettigs& settings)
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

  const RenderTargetSettigs& RenderTarget::GetSettings() const { return m_settings; }

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
