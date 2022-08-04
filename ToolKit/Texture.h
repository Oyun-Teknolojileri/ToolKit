#pragma once

#include <vector>

#include "Types.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Renderer.h"

namespace ToolKit
{
  class Viewport;
  class Renderer;

  class TK_API Texture : public Resource
  {
   public:
    TKResourceType(Texture)

    explicit Texture(bool floatFormat = false);
    explicit Texture(String file, bool floatFormat = false);
    explicit Texture(uint textureId);
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
    bool m_floatFormat = false;
    uint8* m_image = nullptr;
    float* m_imagef = nullptr;
  };

  class TK_API CubeMap : public Texture
  {
   public:
    TKResourceType(CubeMap)

    CubeMap();
    explicit CubeMap(String file);
    explicit CubeMap(uint cubemapId);
    ~CubeMap();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;

   protected:
    void Clear() override;

   public:
    std::vector<uint8*> m_images;
  };

  struct CubeMapSettings
  {
    int level;
    int internalformat;
    int width;
    int height;
    uint format;
    uint type;
    void* pixels;
    int wrapSet;
    int filterSet;
  };

  class TK_API Hdri : public Texture
  {
   public:
    TKResourceType(Hdri);

    Hdri();
    explicit Hdri(const String& file);
    virtual ~Hdri();

    void Load() override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;

    bool IsTextureAssigned();
    uint GetCubemapId();
    void SetCubemapId(uint id);
    uint GetIrradianceCubemapId();
    void SetIrradianceCubemapId(uint id);

   protected:
    void CreateFramebuffer();
    void GenerateCubemapFrom2DTexture();
    void GenerateIrradianceMap();

   private:
    uint GenerateCubemapBuffers(struct CubeMapSettings cubeMapSettings);
    void RenderToCubeMap
    (
      int fbo,
      const Mat4 views[6],
      CameraPtr cam,
      uint cubeMapTextureId,
      int width,
      int height,
      MaterialPtr mat
    );

   public:
    CubeMapPtr m_cubemap = nullptr;
    CubeMapPtr m_irradianceCubemap = nullptr;
    float m_exposure = 1.0f;

   protected:
    uint m_captureFBO = 0;
    uint m_captureRBO = 0;

    MaterialPtr m_texToCubemapMat = nullptr;
    MaterialPtr m_cubemapToIrradiancemapMat = nullptr;
    TexturePtr m_equirectangularTexture = nullptr;
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
