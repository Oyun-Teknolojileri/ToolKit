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
    BoundingBox GetAABB(bool inWorld = false) const override;

   protected:
    virtual void ParameterConstructor();
    virtual void ParameterEventConstructor();

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
    CubeMapPtr GetIrradianceMap() override; // TODO(Osman)

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;

   public:
    TKDeclareParam(float, Exposure);
    TKDeclareParam(HdriPtr, Hdri);
  };

  class TK_API GradientSky : public SkyBase
  {
   public:
    GradientSky();
    virtual ~GradientSky();

    EntityType GetType() const override;

    void Init() override;

    MaterialPtr GetSkyboxMaterial() override;
    CubeMapPtr GetIrradianceMap() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void GenerateGradientCubemap();
    void GenerateIrradianceCubemap();

   public:
    TKDeclareParam(Vec3, TopColor);
    TKDeclareParam(Vec3, MiddleColor);
    TKDeclareParam(Vec3, BottomColor);
    TKDeclareParam(float, GradientExponent);
    TKDeclareParam(float, IrradianceResolution);
    uint m_size = 1024;

   private:
    CubeMapPtr m_skyboxMap                  = nullptr;
    CubeMapPtr m_irradianceMap              = nullptr;
    bool m_onInit                           = false;
  };

} // namespace ToolKit
