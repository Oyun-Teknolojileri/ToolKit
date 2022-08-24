
#include "Light.h"

#include <string>

#include "Component.h"
#include "DirectionComponent.h"

namespace ToolKit
{
  Light::Light()
  {
    Color_Define(Vec3(1.0f), "Light", 0, true, true, { true });
    Intensity_Define(1.0f, "Light", 90, true, true);
  }

  Light::~Light()
  {
  }

  EntityType Light::GetType() const
  {
    return EntityType::Entity_Light;
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

  DirectionalLight::DirectionalLight()
  {
    AddComponent(new DirectionComponent(this));
  }

  EntityType DirectionalLight::GetType() const
  {
    return EntityType::Entity_DirectionalLight;
  }

  PointLight::PointLight()
  {
    Radius_Define(3.0f, "Light", 90, true, true);
  }

  EntityType PointLight::GetType() const
  {
    return EntityType::Entity_PointLight;
  }

  SpotLight::SpotLight()
  {
    Radius_Define(10.0f, "Light", 90, true, true);
    OuterAngle_Define(35.0f, "Light", 90, true, true);
    InnerAngle_Define(30.0f, "Light", 90, true, true);

    AddComponent(new DirectionComponent(this));
  }

  EntityType SpotLight::GetType() const
  {
    return EntityType::Entity_SpotLight;
  }
}  // namespace ToolKit
