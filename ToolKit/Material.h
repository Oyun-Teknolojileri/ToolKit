#pragma once

#include "RenderState.h"
#include "Resource.h"
#include "ResourceManager.h"
#include "Renderer.h"

namespace ToolKit
{

  class TK_API Material : public Resource
  {
   public:
    TKResouceType(Material)

    Material();
    explicit Material(String file);
    ~Material();

    void Load() override;
    void Save(bool onlyIfDirty) override;
    void Init(bool flushClientSideArray = true) override;
    void UnInit() override;
    RenderState* GetRenderState();

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   private:
    void CopyTo(Resource* other) override;

   public:
    CubeMapPtr m_cubeMap;
    TexturePtr m_diffuseTexture;
    ShaderPtr m_vertexShader;
    ShaderPtr m_fragmetShader;
    Vec3 m_color;
    float m_alpha;

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
    MaterialPtr GetCopyOfSolidMaterial();
    MaterialPtr GetCopyOfDefaultMaterial();
  };

}  // namespace ToolKit
