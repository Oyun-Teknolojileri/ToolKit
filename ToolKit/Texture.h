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
    GraphicTypes Target         = GraphicTypes::Target2D;
    GraphicTypes WarpS          = GraphicTypes::UVRepeat;
    GraphicTypes WarpT          = GraphicTypes::UVRepeat;
    GraphicTypes WarpR          = GraphicTypes::UVRepeat;
    GraphicTypes MinFilter      = GraphicTypes::SampleNearest;
    GraphicTypes MagFilter      = GraphicTypes::SampleNearest;
    GraphicTypes InternalFormat = GraphicTypes::FormatRGBA16F;
    GraphicTypes Format         = GraphicTypes::FormatRGBA;
    GraphicTypes Type           = GraphicTypes::TypeFloat;
    int Layers                  = 0;     //!< Number of layers that this texture have if this is a texture array.
    bool GenerateMipMap         = false; //!< Generates mipmaps for the texture automatically.

    bool operator==(const TextureSettings& other) const { return memcmp(this, &other, sizeof(TextureSettings)) == 0; }

    bool operator!=(const TextureSettings& other) const { return !(*this == other); }
  };

  // Texture
  //////////////////////////////////////////////////////////////////////////

  class TK_API Texture : public Resource
  {
   public:
    TKDeclareClass(Texture, Resource);

    Texture();
    Texture(const String& file);
    virtual ~Texture();
    virtual void NativeConstruct(int widht, int height, const TextureSettings& settings);

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

    const TextureSettings& Settings();
    void Settings(const TextureSettings& settings);

   protected:
    virtual void Clear();

   public:
    uint m_textureId  = 0;
    int m_width       = 0;
    int m_height      = 0;
    int m_numChannels = 0; //!< Number of channels (r, g, b, a) for loaded images.
    uint8* m_image    = nullptr;
    float* m_imagef   = nullptr;

   protected:
    TextureSettings m_settings;
  };

  // DepthTexture
  //////////////////////////////////////////////////////////////////////////

  class TK_API DepthTexture : public Texture
  {
   public:
    TKDeclareClass(DepthTexture, Texture);

   public:
    void Load() override;
    void Init(int width, int height, bool stencil);
    void UnInit() override;

    GraphicTypes GetDepthFormat();

   protected:
    void Clear() override;

   public:
    bool m_stencil;
  };

  // DataTexture
  //////////////////////////////////////////////////////////////////////////

  class TK_API DataTexture : public Texture
  {
   public:
    TKDeclareClass(DataTexture, Texture);

   public:
    void Load() override;
    void Init(void* data);
    void UnInit() override;
  };

  // CubeMap
  //////////////////////////////////////////////////////////////////////////

  class TK_API CubeMap : public Texture
  {
   public:
    TKDeclareClass(CubeMap, Texture);

   public:
    CubeMap();
    CubeMap(const String& file);
    virtual ~CubeMap();

    /**
     * Takes the ownership of a render target. Simply to use the render to cube map results as cube map.
     * @param cubeMapTarget is the cube map render target to be consumed. It can be safely destroyed after consumed.
     */
    void Consume(RenderTargetPtr cubeMapTarget);

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

   protected:
    void Clear() override;

   public:
    std::vector<uint8*> m_images;
  };

  // Hdri
  //////////////////////////////////////////////////////////////////////////

  class TK_API Hdri : public Texture
  {
   public:
    TKDeclareClass(Hdri, Texture);

   public:
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

   private:
    bool m_waitingForInit = false;
  };

  // RenderTarget
  //////////////////////////////////////////////////////////////////////////

  class TK_API RenderTarget : public Texture
  {
   public:
    TKDeclareClass(RenderTarget, Texture);

   public:
    RenderTarget();
    virtual ~RenderTarget();

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void Reconstruct(int width, int height, const TextureSettings& settings);
    void ReconstructIfNeeded(int width, int height, const TextureSettings* settings = nullptr);
  };

  // TextureManager
  //////////////////////////////////////////////////////////////////////////

  class TK_API TextureManager : public ResourceManager
  {
   public:
    TextureManager();
    virtual ~TextureManager();
    bool CanStore(ClassMeta* Class) override;
    String GetDefaultResource(ClassMeta*) override;
  };

} // namespace ToolKit
