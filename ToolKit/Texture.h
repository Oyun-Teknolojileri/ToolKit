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
    TKResourceType(Texture)

    Texture();
    explicit Texture(String file);
    virtual ~Texture();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;

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
    TKResourceType(CubeMap)

    CubeMap();
    explicit CubeMap(String file);
    ~CubeMap();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;

   protected:
    void Clear() override;

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
    TKResourceType(RenderTarget)

    RenderTarget();
    RenderTarget
    (
      uint widht,
      uint height,
      const RenderTargetSettigs& settings = RenderTargetSettigs()
    );
    virtual ~RenderTarget();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;
    void Reconstrcut
    (
      uint width,
      uint height,
      const RenderTargetSettigs& settings
    );
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
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
    String GetDefaultResource(ResourceType type) override;
  };

}  // namespace ToolKit
