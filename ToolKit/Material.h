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
    Material(String file);
    ~Material();

    virtual void Load() override;
    virtual void Save(bool onlyIfDirty) override;
    virtual void Init(bool flushClientSideArray = true) override;
    virtual void UnInit() override;
    RenderState* GetRenderState();

    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  private:
    virtual void CopyTo(Resource* other) override;

  public:
    CubeMapPtr m_cubeMap;
    TexturePtr m_diffuseTexture;
    ShaderPtr m_vertexShader;
    ShaderPtr m_fragmetShader;
    Vec3 m_color;
    float m_alpha;

    RenderTechnique m_renderTechnique;

  private:
    RenderState m_renderState;
  };

  class TK_API MaterialManager : public ResourceManager
  {
  public:
    MaterialManager();
    virtual ~MaterialManager();
    virtual void Init() override;
    virtual bool CanStore(ResourceType t);
    virtual ResourcePtr CreateLocal(ResourceType type);
    virtual String GetDefaultResource(ResourceType type) override;

    MaterialPtr GetCopyOfUnlitMaterial();
    MaterialPtr GetCopyOfUnlitColorMaterial();
    MaterialPtr GetCopyOfSolidMaterial();
    MaterialPtr GetCopyOfDefaultMaterial();
  };

}