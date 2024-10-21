/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "DirectionComponent.h"

#include "Entity.h"

namespace ToolKit
{

  TKDefineClass(DirectionComponent, Component);

  DirectionComponent::DirectionComponent() {}

  DirectionComponent::~DirectionComponent() {}

  ComponentPtr DirectionComponent::Copy(EntityPtr ntt)
  {
    DirectionComponentPtr dc = MakeNewPtr<DirectionComponent>();
    dc->m_entity             = ntt;
    return dc;
  }

  Vec3 DirectionComponent::GetDirection() const { return -glm::column(GetOwnerWorldTransform(), 2); }

  void DirectionComponent::Pitch(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(1.0f, 0.0f, 0.0f));
    OwnerEntity()->m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionComponent::Yaw(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 1.0f, 0.0f));
    OwnerEntity()->m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionComponent::Roll(float angle)
  {
    Quaternion q = glm::angleAxis(angle, Vec3(0.0f, 0.0f, 1.0f));
    OwnerEntity()->m_node->Rotate(q, TransformationSpace::TS_LOCAL);
  }

  void DirectionComponent::RotateOnUpVector(float angle)
  {
    OwnerEntity()->m_node->Rotate(glm::angleAxis(angle, Vec3(0.0f, 1.0f, 0.0f)), TransformationSpace::TS_WORLD);
  }

  Vec3 DirectionComponent::GetUp() const { return glm::column(GetOwnerWorldTransform(), 1); }

  Vec3 DirectionComponent::GetRight() const
  {
    Mat4 transform = OwnerEntity()->m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::column(transform, 0);
  }

  void DirectionComponent::LookAt(const Vec3& target)
  {
    Mat4 worldTs   = GetOwnerWorldTransform();
    Vec3 pos       = glm::column(worldTs, 3); // Owner's current position

    Vec3 targetDir = -glm::normalize(target - pos); // Direction to target

    // Calculate right and up vectors using cross products.
    Vec3 up, right;
    float dotVal = glm::abs(glm::dot(Y_AXIS, targetDir));
    if (glm::epsilonEqual(dotVal, 1.0f, 0.0001f)) // Check if up and target collinear.
    {
      right = glm::normalize(GetRight());
      up    = glm::normalize(glm::cross(targetDir, right));
    }
    else
    {
      right = glm::normalize(glm::cross(Y_AXIS, targetDir));
      up    = glm::normalize(glm::cross(targetDir, right));
    }

    // Construct orientation matrix
    Mat3 orientation = Mat3(right, up, targetDir);

    // Set the orientation using a quaternion
    OwnerEntity()->m_node->SetOrientation(glm::toQuat(orientation));
  }

  void DirectionComponent::LookAt(const Vec3& eye, const Vec3& target)
  {
    OwnerEntity()->m_node->SetTranslation(eye);
    LookAt(target);
  }

  XmlNode* DirectionComponent::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* root = Super::SerializeImp(doc, parent);
    if (!m_serializableComponent)
    {
      return root;
    }

    XmlNode* node = CreateXmlNode(doc, StaticClass()->Name, root);

    return node;
  }

  Mat4& DirectionComponent::GetOwnerWorldTransform() const
  {
    if (m_spatialCachesInvalidated)
    {
      m_ownerWorldTransformCache = OwnerEntity()->m_node->GetTransform(TransformationSpace::TS_WORLD);
      m_spatialCachesInvalidated = false;
    }

    return m_ownerWorldTransformCache;
  }

} //  namespace ToolKit
