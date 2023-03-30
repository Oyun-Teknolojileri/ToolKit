#pragma once

#include "Light.h"

namespace ToolKit
{

  class TK_API SkyBase : public Entity
  {
   public:
    SkyBase();

    EntityType GetType() const override;

    virtual void Init();
    virtual void ReInit();
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    bool IsInitialized();

    virtual MaterialPtr GetSkyboxMaterial();
    virtual CubeMapPtr GetIrradianceMap();
    HdriPtr GetHdri();
    BoundingBox GetAABB(bool inWorld = false) const override;

   protected:
    virtual void ParameterConstructor();
    virtual void ParameterEventConstructor();
    void ConstructSkyMaterial(ShaderPtr vertexPrg, ShaderPtr fragPrg);

   public:
    TKDeclareParam(bool, DrawSky);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);

   protected:
    bool m_initialized           = false;
    MaterialPtr m_skyboxMaterial = nullptr;
  };

  class TK_API Sky : public SkyBase
  {
   public:
    Sky();
    virtual ~Sky();

    EntityType GetType() const override;
    void Init() override;
    MaterialPtr GetSkyboxMaterial() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;

   public:
    TKDeclareParam(float, Exposure);
    TKDeclareParam(HdriPtr, Hdri);
  };

} // namespace ToolKit
