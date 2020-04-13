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
    Node(Vec3 translation);
    void Translate(Vec3 val, TransformationSpace space = TransformationSpace::TS_PARENT);
    void Rotate(Quaternion val, TransformationSpace space = TransformationSpace::TS_PARENT);
    void Scale(Vec3 val);
    void Transform(Mat4 val, TransformationSpace space = TransformationSpace::TS_PARENT);
    Mat4 GetTransform(TransformationSpace space = TransformationSpace::TS_WORLD);
    Vec3 GetTranslation(TransformationSpace space = TransformationSpace::TS_PARENT);
    Quaternion GetOrientation(TransformationSpace space = TransformationSpace::TS_PARENT);
    Vec3 GetScale(TransformationSpace space = TransformationSpace::TS_PARENT);
    void AddChild(Node* child);

  public:
    Vec3 m_translation;
    Vec3 m_scale;
    Quaternion m_orientation;
    std::vector<Node*> m_children;
    Node* m_parent;
  };

}