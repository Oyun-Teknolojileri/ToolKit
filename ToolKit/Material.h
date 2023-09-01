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

#include "RenderState.h"
#include "Resource.h"
#include "Texture.h"

namespace ToolKit
{

  enum class MaterialType
  {
    UNUSEDSLOT_1 = 0,
    PBR          = 1,
    Custom       = 2
  };

  class TK_API Material : public Resource
  {
   public:
    TKDeclareClass(Material, Resource);

    Material();
    explicit Material(const String& file);
    ~Material();

    void Load() override;
    void Save(bool onlyIfDirty) override;
    void Init(bool flushClientSideArray = false) override;
    void UnInit() override;
    RenderState* GetRenderState();
    void SetRenderState(RenderState* state);
    void SetDefaultMaterialTypeShaders();

    /**
     * Acces to alpha value (opacity)
     * @returns Referance to alpha value
     */
    float& GetAlpha();

    /**
     * Set the alpha value (opacity)
     */
    void SetAlpha(float val);

    /**
     * States if the material will use deferred render path.
     * @returns True if the material will be rendered in deferred path.
     */
    bool IsDeferred();

    /**
     * States if the material has transparency.
     * @returns True if the blend state is SRC_ALPHA_ONE_MINUS_SRC_ALPHA.
     */
    bool IsTranslucent();

    /**
     * States if the material is using PBR shaders.
     * @returns True if the fragmet shader is default PBR shader.
     */
    bool IsPBR();

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   private:
    void CopyTo(Resource* other) override;

   public:
    CubeMapPtr m_cubeMap;
    TexturePtr m_diffuseTexture;
    TexturePtr m_emissiveTexture;
    TexturePtr m_metallicRoughnessTexture;
    TexturePtr m_normalMap;
    ShaderPtr m_vertexShader;
    ShaderPtr m_fragmentShader;
    Vec3 m_color;
    Vec3 m_emissiveColor;
    float m_metallic            = 0.2f;
    float m_roughness           = 0.5f;

    MaterialType m_materialType = MaterialType::Custom;

   private:
    float m_alpha = 1.0f;
    RenderState m_renderState;
  };

  class TK_API MaterialManager : public ResourceManager
  {
   public:
    MaterialManager();
    virtual ~MaterialManager();
    void Init() override;
    bool CanStore(TKClass* Class) override;
    ResourcePtr CreateLocal(TKClass* Class) override;
    String GetDefaultResource(TKClass* Class) override;

    MaterialPtr GetCopyOfUnlitMaterial();
    MaterialPtr GetCopyOfUIMaterial();
    MaterialPtr GetCopyOfUnlitColorMaterial();
    MaterialPtr GetCopyOfDefaultMaterial();
  };

} // namespace ToolKit
