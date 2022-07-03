#include "DirectionComponent.h"
#include "Entity.h"

namespace ToolKit
{

  DirectionComponent::DirectionComponent()
  {
  }

  DirectionComponent::DirectionComponent(Entity* entity)
  {
    m_entity = entity;
  }

  DirectionComponent::~DirectionComponent()
  {
  }

  ComponentPtr DirectionComponent::Copy(Entity* ntt)
  {
    DirectionComponentPtr dc =
    std::make_shared<DirectionComponent>(m_entity);
    dc->m_entity = ntt;
    return dc;
  }

  Vec3 DirectionComponent::GetDirection()
  {
    Mat4 transform =
    m_entity->m_node->GetTransform(TransformationSpace::TS_WORLD);
    return -glm::column(transform, 2);
  }

  void DirectionComponent::Pitch(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(1.0f, 0.0f, 0.0f));
    m_entity->m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionComponent::Yaw(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 1.0f, 0.0f));
    m_entity->m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionComponent::Roll(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 0.0f, 1.0f));
    m_entity->m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionComponent::RotateOnUpVector(float angle)
  {
    m_entity->m_node->Rotate
    (
      glm::angleAxis
      (
        angle,
        Vec3(0.0f, 1.0f, 0.0f)
      ),
      TransformationSpace::TS_WORLD
    );
  }

  Vec3 DirectionComponent::GetUp() const
  {
    Mat4 transform =
    m_entity->m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 1);
  }

  Vec3 DirectionComponent::GetRight() const
  {
    Mat4 transform =
    m_entity->m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 0);
  }

  void DirectionComponent::LookAt(Vec3 target)
  {
    Vec3 eye = m_entity->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    Vec3 tdir = target - eye;
    tdir.y = 0.0f;  // project on xz
    tdir = glm::normalize(tdir);
    Vec3 dir = GetDirection();
    dir.y = 0.0f;  // project on xz
    dir = glm::normalize(dir);

    if (glm::all(glm::epsilonEqual(tdir, dir, { 0.0001f, 0.0001f, 0.0001f })))
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

}  //  namespace ToolKit
