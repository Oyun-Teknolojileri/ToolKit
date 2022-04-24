#include "stdafx.h"
#include "Texture.h"
#include "ToolKit.h"
#include "GL/glew.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "DebugNew.h"

namespace ToolKit
{

  Texture::Texture()
  {
    m_textureId = 0;
  }

  Texture::Texture(String file)
    : Texture()
  {
    SetFile(file);
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

    if 
    (
      m_image = stbi_load
      (
        GetFile().c_str(),
        &m_width,
        &m_height,
        &m_bytePP,
        4
      )
    )
    {
      m_loaded = true;
    }
  }

  void Texture::Init(bool flushClientSideArray)
  {
    if (m_initiated)
    {
      return;
    }

    if (m_image == nullptr || m_width <= 0 || m_height <= 0)
    {
      return;
    }

    glGenTextures(1, &m_textureId);

    GLint currId;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);

    glBindTexture(GL_TEXTURE_2D, m_textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
    m_image = nullptr;
  }

  CubeMap::CubeMap()
    : Texture()
  {
  }

  CubeMap::CubeMap(String file)
    : Texture()
  {
    SetFile(file);
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
    size_t pos = fullPath.find("px.png");
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
      if ((m_images[i] = stbi_load(name.c_str(), &m_width, &m_height, &m_bytePP, 0)))
      {
        GetLogger()->Log("Missing file: " + name);
        GetLogger()->Log("Cube map loading requires additional 5 png files with postfix \"nx py ny pz nz\".");
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

    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureId);

    uint sides[6] =
    {
      GL_TEXTURE_CUBE_MAP_POSITIVE_X,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
      GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    };

    for (int i = 0; i < 6; i++)
    {
      glTexImage2D(sides[i], 0, GL_RGBA, m_width, m_width, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_images[i]);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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
    for (int i = 0; i < 6; i++)
    {
      stbi_image_free(m_images[i]);
      m_images[i] = nullptr;
    }
  }

  RenderTarget::RenderTarget()
    : Texture()
  {
  }

  RenderTarget::RenderTarget(uint width, uint height, const RenderTargetSettigs& settings)
    : RenderTarget()
  {
    m_width = width;
    m_height = height;
    m_frameBufferId = 0;
    m_depthBufferId = 0;
    m_settings = settings;
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

    GLint currId; // Don't override the current render target.
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currId);
    
    // Create frame buffer color texture
    glGenTextures(1, &m_textureId);
    glBindTexture(GL_TEXTURE_2D, m_textureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (int)m_settings.WarpS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (int)m_settings.WarpT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)m_settings.MinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)m_settings.MagFilter);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glGenFramebuffers(1, &m_frameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_frameBufferId);

    // Attach 2D texture to this FBO
    bool msaaRTunsupported = false;
    if (glFramebufferTexture2DMultisampleEXT == nullptr)
    {
      msaaRTunsupported = true;
      
      static bool notReported = true;
      if (notReported)
      {
        GetLogger()->Log("Unsupported Extension: glFramebufferTexture2DMultisampleEXT");
        notReported = false;
      }
    }

    bool goForMsaa = m_settings.Msaa > 0 && !msaaRTunsupported;
    if (goForMsaa)
    {
      glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0, m_settings.Msaa);
    }
    else
    {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureId, 0);
    }

    if (m_settings.DepthStencil)
    {
      glGenRenderbuffers(1, &m_depthBufferId);
      glBindRenderbuffer(GL_RENDERBUFFER, m_depthBufferId);

      if (goForMsaa)
      {
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, m_settings.Msaa, GL_DEPTH24_STENCIL8, m_width, m_height);
      }
      else
      {
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
      }

      // Attach depth buffer to FBO
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthBufferId);
    }

    if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
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
    glBindTexture(GL_TEXTURE_2D, currId); // Restore previous render target.
  }

  void RenderTarget::UnInit()
  {
    Texture::UnInit();

    glDeleteFramebuffers(1, &m_frameBufferId);
    glDeleteRenderbuffers(1, &m_depthBufferId);
    m_initiated = false;
  }

  void RenderTarget::Reconstrcut(uint width, uint height, const RenderTargetSettigs& settings)
  {
    UnInit();
    m_width = width;
    m_height = height;
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
    if
    (
      t == ResourceType::Texture
      || t == ResourceType::CubeMap
      || t == ResourceType::RenderTarget
    )
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
    default:
      assert(false);
      break;
    }
      return ResourcePtr(tex);
  }

  String TextureManager::GetDefaultResource(ResourceType type)
  {
    return TexturePath("default.png", true);
  }

}
