#pragma once

#include "RenderState.h"
#include "Renderer.h"
#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{

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

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   private:
    void CopyTo(Resource* other) override;

   public:
    CubeMapPtr m_cubeMap;
    TexturePtr m_diffuseTexture;
    TexturePtr m_emissiveTexture;
    ShaderPtr m_vertexShader;
    ShaderPtr m_fragmentShader;
    Vec3 m_color;
    Vec3 m_emissiveColor;
    float m_alpha = 1.0f;

   private:
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
