#pragma once

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "GL\glew.h"
#include <vector>

namespace ToolKit
{

  class Texture : public Resource
  {
  public:
    Texture();
    Texture(String file);
    virtual ~Texture();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

  protected:
    virtual void Clear();

  public:
    GLuint m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
    int m_bytePP = 0;
    uint8* m_image = nullptr;
  };

  class CubeMap : public Texture
  {
  public:
    CubeMap();
    CubeMap(String file);
    ~CubeMap();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

  protected:
    virtual void Clear() override;

  public:
    std::vector<uint8*> m_images;
  };

  class RenderTargetSettigs
  {
  public:
    bool depthStencil = true;
    int warpS = GL_REPEAT;
    int warpT = GL_REPEAT;
    int minFilter = GL_NEAREST;
    int magFilter = GL_NEAREST;
  };

  class RenderTarget : public Texture
  {
  public:
    RenderTarget();
    RenderTarget(uint widht, uint height, const RenderTargetSettigs& settings = RenderTargetSettigs());
    virtual ~RenderTarget();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

  public:
    GLuint m_frameBufferId;
    GLuint m_depthBufferId;

  private:
    RenderTargetSettigs m_settings;
  };

  class TextureManager : public ResourceManager
  {
  public:
    TextureManager();
    virtual ~TextureManager();
    virtual bool CanStore(ResourceType t);
  };

}