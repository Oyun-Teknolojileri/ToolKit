
#include "Light.h"
#include "Component.h"
#include "DirectionComponent.h"

#include <string>


namespace ToolKit
{
  Light::Light()
  {
    m_localData.m_variants.reserve(m_localData.m_variants.size() + 3);

    LightType_Define(0, "Light", 90, false, false);
    Color_Define(Vec3(1.0f), "Light", 90, true, true);
    Intensity_Define(1.0f, "Light", 90, true, true);
  }

  Light::~Light()
  {
  }

  EntityType Light::GetType() const
  {
    return EntityType::Entity_Light;
  }

  LightTypeEnum Light::GetLightType() const
  {
    return LightTypeEnum::LightBase;
  }

  void Light::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
  }

  void Light::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents();  // Read from file.
    Entity::DeSerialize(doc, parent);
  }

  void Light::Init()
  {
  }

  DirectionalLight::DirectionalLight()
  {
    SetLightTypeVal(static_cast<int> (LightTypeEnum::LightDirectional));
    AddComponent(new DirectionComponent(this));
  }

  LightTypeEnum DirectionalLight::GetLightType() const
  {
    return LightTypeEnum::LightDirectional;
  }

  PointLight::PointLight()
  {
    m_localData.m_variants.reserve(m_localData.m_variants.size() + 1);

    SetLightTypeVal(static_cast<int> (LightTypeEnum::LightPoint));
    Radius_Define(3.0f, "Light", 90, true, true);
  }

  LightTypeEnum PointLight::GetLightType() const
  {
    return LightTypeEnum::LightPoint;
  }

  SpotLight::SpotLight()
  {
    m_localData.m_variants.reserve(m_localData.m_variants.size() + 3);
    SetLightTypeVal(static_cast<int> (LightTypeEnum::LightSpot));
    Radius_Define(10.0f, "Light", 90, true, true);
    OuterAngle_Define(35.0f, "Light", 90, true, true);
    InnerAngle_Define(30.0f, "Light", 90, true, true);

    AddComponent(new DirectionComponent(this));
  }

  LightTypeEnum SpotLight::GetLightType() const
  {
    return LightTypeEnum::LightSpot;
  }
}  // namespace ToolKit
