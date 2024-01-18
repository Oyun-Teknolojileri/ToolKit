/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "RenderState.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Texture.h"

namespace ToolKit
{
  class TK_API Material : public Resource
  {
   public:
    TKDeclareClass(Material, Resource);

    Material();
    explicit Material(const String& file);
    virtual ~Material();

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
    bool m_updateGPUUniforms = true;

    CubeMapPtr m_cubeMap;
    TexturePtr m_diffuseTexture;
    TexturePtr m_emissiveTexture;
    TexturePtr m_metallicRoughnessTexture;
    TexturePtr m_normalMap;
    ShaderPtr m_vertexShader;
    ShaderPtr m_fragmentShader;
    Vec3 m_color;
    Vec3 m_emissiveColor;
    float m_metallic  = 0.2f;
    float m_roughness = 0.5f;

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
    bool CanStore(ClassMeta* Class) override;
    String GetDefaultResource(ClassMeta* Class) override;

    MaterialPtr GetCopyOfUnlitMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfUIMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfUnlitColorMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfDefaultMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfPhongMaterial(bool storeInMaterialManager = true);
  };

} // namespace ToolKit
