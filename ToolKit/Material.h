#pragma once

#include "RenderState.h"
#include "Renderer.h"
#include "Resource.h"
#include "ResourceManager.h"

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
    TKResourceType(Material)

    Material();
    explicit Material(String file);
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

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

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
    float m_alpha               = 1.0f;
    RenderState m_renderState;
  };

  class TK_API MaterialManager : public ResourceManager
  {
   public:
    MaterialManager();
    virtual ~MaterialManager();
    void Init() override;
    bool CanStore(ResourceType t) override;
    ResourcePtr CreateLocal(ResourceType type) override;
    String GetDefaultResource(ResourceType type) override;

    MaterialPtr GetCopyOfUnlitMaterial();
    MaterialPtr GetCopyOfUIMaterial();
    MaterialPtr GetCopyOfUnlitColorMaterial();
    MaterialPtr GetCopyOfDefaultMaterial();
  };

} // namespace ToolKit
