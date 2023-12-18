/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Resource.h"
#include "ResourceManager.h"
#include "Types.h"

namespace ToolKit
{
  struct TextureSettings
  {
    GraphicTypes MinFilter       = GraphicTypes::SampleLinear;
    GraphicTypes InternalFormat  = GraphicTypes::FormatSRGB8_A8;
    GraphicTypes Type            = GraphicTypes::TypeUnsignedByte;
    GraphicTypes MipMapMinFilter = GraphicTypes::SampleLinearMipmapLinear;
    bool GenerateMipMap          = true;
    GraphicTypes Target          = GraphicTypes::Target2D;
    int Layers                   = -1;
  };

  class TK_API Texture : public Resource
  {
   public:
    TKDeclareClass(Texture, Resource);

    Texture(const TextureSettings& settings = {});
    Texture(const String& file, const TextureSettings& settings = {});
    virtual ~Texture();

    virtual void NativeConstruct(uint textureId);

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

    const TextureSettings& GetTextureSettings();
    void SetTextureSettings(const TextureSettings& settings);

   protected:
    virtual void Clear();

   public:
    uint m_textureId = 0;
    int m_width      = 0;
    int m_height     = 0;
    int m_bytePP     = 0;
    uint8* m_image   = nullptr;
    float* m_imagef  = nullptr;

   protected:
    TextureSettings m_textureSettings;
  };

  class DepthTexture : public Texture
  {
   public:
    TKDeclareClass(DepthTexture, Texture);

    void Load() override;
    void Init(int width, int height, bool stencil);
    void UnInit() override;

   protected:
    void Clear() override;

   public:
    bool m_stencil;
  };

  typedef std::shared_ptr<DepthTexture> DepthTexturePtr;

  class TK_API CubeMap : public Texture
  {
   public:
    TKDeclareClass(CubeMap, Texture);

    CubeMap();
    CubeMap(const String& file);
    virtual ~CubeMap();

    using Texture::NativeConstruct;

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

   protected:
    void Clear() override;

   public:
    std::vector<uint8*> m_images;
  };

  class TK_API Hdri : public Texture
  {
   public:
    TKDeclareClass(Hdri, Texture);

    Hdri();
    explicit Hdri(const String& file);
    virtual ~Hdri();

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

    bool IsTextureAssigned();

   public:
    CubeMapPtr m_cubemap         = nullptr;
    CubeMapPtr m_specularEnvMap  = nullptr;
    CubeMapPtr m_diffuseEnvMap   = nullptr;
    float m_exposure             = 1.0f;
    int m_specularIBLTextureSize = 256;

   protected:
    MaterialPtr m_texToCubemapMat           = nullptr;
    MaterialPtr m_cubemapToDiffuseEnvMapMat = nullptr;
    TexturePtr m_equirectangularTexture     = nullptr;

    bool m_waitingForInit                   = false;
  };

  struct RenderTargetSettigs
  {
    byte Msaa                   = 0;
    GraphicTypes Target         = GraphicTypes::Target2D;
    GraphicTypes WarpS          = GraphicTypes::UVRepeat;
    GraphicTypes WarpT          = GraphicTypes::UVRepeat;
    GraphicTypes WarpR          = GraphicTypes::UVRepeat;
    GraphicTypes MinFilter      = GraphicTypes::SampleNearest;
    GraphicTypes MagFilter      = GraphicTypes::SampleNearest;
    GraphicTypes InternalFormat = GraphicTypes::FormatRGBA16F;
    GraphicTypes Format         = GraphicTypes::FormatRGBA;
    GraphicTypes Type           = GraphicTypes::TypeFloat;
    int Layers                  = 1;
  };

  class TK_API RenderTarget : public Texture
  {
   public:
    TKDeclareClass(RenderTarget, Texture);

    RenderTarget();
    virtual ~RenderTarget();
    virtual void NativeConstruct(uint widht, uint height, const RenderTargetSettigs& settings = RenderTargetSettigs());

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void Reconstruct(uint width, uint height, const RenderTargetSettigs& settings);
    void ReconstructIfNeeded(uint width, uint height);
    const RenderTargetSettigs& GetSettings() const;

   public:
    RenderTargetSettigs m_settings;
  };

  class TK_API TextureManager : public ResourceManager
  {
   public:
    TextureManager();
    virtual ~TextureManager();
    bool CanStore(ClassMeta* Class) override;
    String GetDefaultResource(ClassMeta*) override;
  };

} // namespace ToolKit
