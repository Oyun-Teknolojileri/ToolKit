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
#include "ShaderUniform.h"
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
     * States if the material has transparency.
     * @returns True if the blend state is SRC_ALPHA_ONE_MINUS_SRC_ALPHA.
     */
    bool IsTranslucent();

    /**
     * @returns True if the material is alpha masked.
     */
    bool IsAlphaMasked();

    /**
     * States if the material is using PBR shaders.
     * @returns True if the fragmet shader is default PBR shader.
     */
    bool IsPBR();

    // This should be called when this material parameter changed except for Textures, Shaders and RenderState
    void UpdateRuntimeVersion();

    inline uint64 GetRuntimeVersion() { return m_uniformVersion; }

    void UpdateProgramUniform(const String& uniformName, const UniformValue& val);

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
    float m_metallic        = 0.2f;
    float m_roughness       = 0.5f;
    bool m_isShaderMaterial = false;

   private:
    float m_alpha = 1.0f;
    RenderState m_renderState;

    /**
     * Represents the GPU uniform state. GpuPrograms update their uniforms if their uniform version is different than
     * current material uniform version.
     */
    uint64 m_uniformVersion = 1;
  };

  class TK_API MaterialManager : public ResourceManager
  {
   public:
    MaterialManager();
    virtual ~MaterialManager();
    void Init() override;
    bool CanStore(ClassMeta* Class) override;
    String GetDefaultResource(ClassMeta* Class) override;
    MaterialPtr GetDefaultMaterial();
    MaterialPtr GetCopyOfUnlitMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfUIMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfUnlitColorMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfDefaultMaterial(bool storeInMaterialManager = true);
    MaterialPtr GetCopyOfPhongMaterial(bool storeInMaterialManager = true);

   private:
    MaterialPtr m_defaultMaterial = nullptr;
    MaterialPtr m_defaultAlphaMaskedMaterial = nullptr;
  };

} // namespace ToolKit
