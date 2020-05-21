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

  public:
    GLuint m_textureId = 0;
    uint m_width = 0;
    uint m_height = 0;
    std::vector<unsigned char> m_image;
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

  public:
    std::vector<std::vector<unsigned char>> m_images;
  };

  class RenderTarget : public Texture
  {
  public:
    RenderTarget(uint widht, uint height, bool depthStencil = true);
		virtual ~RenderTarget();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;

	public:
		GLuint m_frameBufferId;
		GLuint m_depthBufferId;

  private:
    bool m_depthStencil;
  };

  class TextureManager : public ResourceManager<Texture>
  {
  };

}