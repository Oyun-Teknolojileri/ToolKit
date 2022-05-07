
#include "Light.h"

#include <string>


namespace ToolKit
{

  Light::Light()
  {
    m_lightData.type = 0;
    m_lightData.color = Vec3(1.0f);
    m_lightData.intensity = 1.0f;
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
    parent = parent->last_node();

    XmlNode* node = doc->allocate_node(rapidxml::node_element, "Light");
    parent->append_node(node);

    WriteAttr(node, doc, "ty", std::to_string(m_lightData.type));
    WriteAttr(node, doc, "cR", std::to_string(m_lightData.color.x));
    WriteAttr(node, doc, "cG", std::to_string(m_lightData.color.y));
    WriteAttr(node, doc, "cB", std::to_string(m_lightData.color.z));
    WriteAttr(node, doc, "int", std::to_string(m_lightData.intensity));
    WriteAttr(node, doc, "ra", std::to_string(m_lightData.radius));
    WriteAttr(node, doc, "outAng", std::to_string(m_lightData.outerAngle));
    WriteAttr(node, doc, "inAng", std::to_string(m_lightData.innerAngle));
  }

  void Light::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    if (XmlNode* node = parent->first_node("Light"))
    {
      ReadAttr(node, "ty", m_lightData.type);
      ReadAttr(node, "cR", m_lightData.color.x);
      ReadAttr(node, "cG", m_lightData.color.y);
      ReadAttr(node, "cB", m_lightData.color.z);
      ReadAttr(node, "int", m_lightData.intensity);
      ReadAttr(node, "ra", m_lightData.radius);
      ReadAttr(node, "outAng", m_lightData.outerAngle);
      ReadAttr(node, "inAng", m_lightData.innerAngle);
    }
  }

  void Light::Init()
  {
  }

  DirectionalLight::DirectionalLight()
  {
    m_lightData.type = 1;
  }

  LightType DirectionalLight::GetLightType() const
  {
    return LightType::LightDirectional;
  }

  Vec3 DirectionalLight::GetDirection()
  {
    Mat4 transform = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return -glm::column(transform, 2);
  }

  void DirectionalLight::Pitch(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(1.0f, 0.0f, 0.0f));
    m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionalLight::Yaw(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 1.0f, 0.0f));
    m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionalLight::Roll(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 0.0f, 1.0f));
    m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionalLight::RotateOnUpVector(float angle)
  {
    m_node->Rotate
    (
      glm::angleAxis
      (
      angle,
      Vec3(0.0f, 1.0f, 0.0f)
    ),
      TransformationSpace::TS_WORLD
    );
  }

  Vec3 DirectionalLight::GetUp() const
  {
    Mat4 transform = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 1);
  }

  Vec3 DirectionalLight::GetRight() const
  {
    Mat4 transform = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 0);
  }

  void DirectionalLight::LookAt(Vec3 target)
  {
    Vec3 eye = m_node->GetTranslation(TransformationSpace::TS_WORLD);
    Vec3 tdir = target - eye;
    tdir.y = 0.0f;  // project on xz
    tdir = glm::normalize(tdir);
    Vec3 dir = GetDirection();
    dir.y = 0.0f;  // project on xz
    dir = glm::normalize(dir);

    if (glm::all(glm::epsilonEqual(tdir, dir, { 0.01f, 0.01f, 0.01f })))
    {
      return;
    }

    Vec3 rotAxis = glm::normalize(glm::cross(dir, tdir));
    float yaw = glm::acos(glm::dot(tdir, dir));

    yaw *= glm::sign(glm::dot(Y_AXIS, rotAxis));
    RotateOnUpVector(yaw);

    tdir = target - eye;
    tdir = glm::normalize(tdir);
    dir = glm::normalize(GetDirection());
    rotAxis = glm::normalize(glm::cross(dir, tdir));
    float pitch = glm::acos(glm::dot(tdir, dir));
    pitch *= glm::sign(glm::dot(GetRight(), rotAxis));
    Pitch(pitch);

    // Check upside down case
    if (glm::dot(GetUp(), Y_AXIS) < 0.0f)
    {
      Roll(glm::pi<float>());
    }
  }

  PointLight::PointLight()
  {
    m_lightData.type = 2;
    m_lightData.radius = 50.0f;
  }

  LightType PointLight::GetLightType() const
  {
    return LightType::LightPoint;
  }

  SpotLight::SpotLight()
  {
    m_lightData.type = 3;
    m_lightData.radius = 10.0f;
    m_lightData.innerAngle = glm::radians(30.0f);
    m_lightData.outerAngle = glm::radians(35.0f);
  }

  LightType SpotLight::GetLightType() const
  {
    return LightType::LightSpot;
  }

  Vec3 SpotLight::GetDirection()
  {
    Mat4 transform = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return -glm::column(transform, 2);
  }

  void SpotLight::Pitch(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(1.0f, 0.0f, 0.0f));
    m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void SpotLight::Yaw(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 1.0f, 0.0f));
    m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void SpotLight::Roll(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 0.0f, 1.0f));
    m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void SpotLight::RotateOnUpVector(float angle)
  {
    m_node->Rotate
    (
      glm::angleAxis
      (
      angle,
      Vec3(0.0f, 1.0f, 0.0f)
    ),
      TransformationSpace::TS_WORLD
    );
  }

  Vec3 SpotLight::GetUp() const
  {
    Mat4 transform = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 1);
  }

  Vec3 SpotLight::GetRight() const
  {
    Mat4 transform = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 0);
  }

  void SpotLight::LookAt(Vec3 target)
  {
    Vec3 eye = m_node->GetTranslation(TransformationSpace::TS_WORLD);
    Vec3 tdir = target - eye;
    tdir.y = 0.0f;  // project on xz
    tdir = glm::normalize(tdir);
    Vec3 dir = GetDirection();
    dir.y = 0.0f;  // project on xz
    dir = glm::normalize(dir);

    if (glm::all(glm::epsilonEqual(tdir, dir, { 0.01f, 0.01f, 0.01f })))
    {
      return;
    }

    Vec3 rotAxis = glm::normalize(glm::cross(dir, tdir));
    float yaw = glm::acos(glm::dot(tdir, dir));

    yaw *= glm::sign(glm::dot(Y_AXIS, rotAxis));
    RotateOnUpVector(yaw);

    tdir = target - eye;
    tdir = glm::normalize(tdir);
    dir = glm::normalize(GetDirection());
    rotAxis = glm::normalize(glm::cross(dir, tdir));
    float pitch = glm::acos(glm::dot(tdir, dir));
    pitch *= glm::sign(glm::dot(GetRight(), rotAxis));
    Pitch(pitch);

    // Check upside down case
    if (glm::dot(GetUp(), Y_AXIS) < 0.0f)
    {
      Roll(glm::pi<float>());
    }
  }

}  // namespace ToolKit
