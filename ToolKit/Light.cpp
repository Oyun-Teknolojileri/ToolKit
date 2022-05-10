
#include "Light.h"
#include "Component.h"

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

  LightType Light::GetLightType() const
  {
    return LightType::LightBase;
  }

  void Light::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
  }

  void Light::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
  }

  void Light::Init()
  {
  }

  void Light::EnableGizmo(bool enable)
  {
  }

  DirectionalLight::DirectionalLight()
  {
    LightType() = 1;

    AddComponent(new DirectionComponent(this));
  }

  LightType DirectionalLight::GetLightType() const
  {
    return LightType::LightDirectional;
  }

  PointLight::PointLight()
  {
    m_localData.m_variants.reserve(m_localData.m_variants.size() + 1);

    LightType() = 2;
    Radius_Define(50.0f, "Light", 90, true, true);
  }

  LightType PointLight::GetLightType() const
  {
    return LightType::LightPoint;
  }

  SpotLight::SpotLight()
  {
    m_localData.m_variants.reserve(m_localData.m_variants.size() + 3);

    LightType() = 3;
    Radius_Define(10.0f, "Light", 90, true, true);
    OuterAngle_Define(35.0f, "Light", 90, true, true);
    InnerAngle_Define(30.0f, "Light", 90, true, true);

    AddComponent(new DirectionComponent(this));
  }

  LightType SpotLight::GetLightType() const
  {
    return LightType::LightSpot;
  }
}  // namespace ToolKit
