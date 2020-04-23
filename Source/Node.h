#pragma once

#include "ToolKit.h"

namespace ToolKit
{

  enum class TransformationSpace
  {
    TS_WORLD,
    TS_PARENT,
    TS_LOCAL
  };

  class Node
  {
  public:
    Node();
    Node(const Vec3& translation);
    void Translate(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    void Rotate(const Quaternion& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    void Scale(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    void Transform(const Mat4& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    void SetTransform(const Mat4& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    Mat4 GetTransform(TransformationSpace space = TransformationSpace::TS_WORLD) const;
    void SetTranslation(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    Vec3 GetTranslation(TransformationSpace space = TransformationSpace::TS_PARENT) const;
    void SetOrientation(const Quaternion& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    Quaternion GetOrientation(TransformationSpace space = TransformationSpace::TS_PARENT) const;
    void SetScale(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
    Vec3 GetScale(TransformationSpace space = TransformationSpace::TS_PARENT) const;
    void AddChild(Node* child);

  public:
    Vec3 m_translation;
    Quaternion m_orientation;
    Vec3 m_scale = Vec3(1.0f);
    Node* m_parent = nullptr;
    std::vector<Node*> m_children;
  };

}