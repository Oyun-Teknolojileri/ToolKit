#pragma once

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include <vector>

namespace ToolKit
{

  class TK_API Texture : public Resource
  {
  public:
    TKResouceType(Texture)

    Texture();
    Texture(String file);
    virtual ~Texture();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;

  protected:
    virtual void Clear();

  public:
    uint m_textureId = 0;
    int m_width = 0;
    int m_height = 0;
    int m_bytePP = 0;
    uint8* m_image = nullptr;
  };

  class TK_API CubeMap : public Texture
  {
  public:
    TKResouceType(CubeMap)

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

  struct RenderTargetSettigs
  {
    byte Msaa = 0;
    bool DepthStencil = true;
    GraphicTypes WarpS = GraphicTypes::UVRepeat;
    GraphicTypes WarpT = GraphicTypes::UVRepeat;
    GraphicTypes MinFilter = GraphicTypes::SampleNearest;
    GraphicTypes MagFilter = GraphicTypes::SampleNearest;
  };

  class TK_API RenderTarget : public Texture
  {
  public:
    TKResouceType(RenderTarget)

    RenderTarget();
    RenderTarget(uint widht, uint height, const RenderTargetSettigs& settings = RenderTargetSettigs());
    virtual ~RenderTarget();

    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;
    void Reconstrcut(uint width, uint height, const RenderTargetSettigs& settings);
    const RenderTargetSettigs& GetSettings() const;

  public:
    uint m_frameBufferId = 0;
    uint m_depthBufferId = 0;

  private:
    RenderTargetSettigs m_settings;
  };

  class TK_API TextureManager : public ResourceManager
  {
  public:
    TextureManager();
    virtual ~TextureManager();
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);
    virtual String GetDefaultResource(ResourceType type) override;
  };

}