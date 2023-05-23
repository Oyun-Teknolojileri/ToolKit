/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "Resource.h"
#include "ResourceManager.h"
#include "Types.h"

#include <vector>

namespace ToolKit
{
  struct TextureSettings
  {
    GraphicTypes MinFilter       = GraphicTypes::SampleLinear;
    GraphicTypes InternalFormat  = GraphicTypes::FormatSRGB8_A8;
    GraphicTypes Type            = GraphicTypes::TypeUnsignedByte;
    GraphicTypes MipMapMinFilter = GraphicTypes::SampleLinearMipmapLinear;
    bool GenerateMipMap          = true;
  };

  class TK_API Texture : public Resource
  {
   public:
    TKResourceType(Texture);

    explicit Texture(const TextureSettings& settings = {});
    explicit Texture(String file, const TextureSettings& settings = {});
    explicit Texture(uint textureId);
    virtual ~Texture();

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

  class TK_API CubeMap : public Texture
  {
   public:
    TKResourceType(CubeMap)

    CubeMap();
    explicit CubeMap(String file);
    explicit CubeMap(uint cubemapId);
    ~CubeMap();

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
    TKResourceType(Hdri);

    Hdri();
    explicit Hdri(const String& file);
    virtual ~Hdri();

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;

    bool IsTextureAssigned();

   protected:
    void GeneratePrefilteredEnvMap();

   public:
    CubeMapPtr m_cubemap           = nullptr;
    CubeMapPtr m_prefilteredEnvMap = nullptr;
    CubeMapPtr m_irradianceCubemap = nullptr;
    float m_exposure               = 1.0f;
    int m_specularIBLTextureSize   = 128;

   protected:
    MaterialPtr m_texToCubemapMat           = nullptr;
    MaterialPtr m_cubemapToIrradiancemapMat = nullptr;
    TexturePtr m_equirectangularTexture     = nullptr;

    const int m_brdfLutTextureSize          = 512;
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
    TKResourceType(RenderTarget)

    RenderTarget();
    RenderTarget(uint widht, uint height, const RenderTargetSettigs& settings = RenderTargetSettigs());
    RenderTarget(Texture* texture);

    void Load() override;
    void Init(bool flushClientSideArray = false) override;
    void Reconstruct(uint width, uint height, const RenderTargetSettigs& settings);
    void ReconstructIfNeeded(uint width, uint height);
    const RenderTargetSettigs& GetSettings() const;

   public:
    RenderTargetSettigs m_settings;

   private:
    TextureSettings m_textureSettings;
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

} // namespace ToolKit
