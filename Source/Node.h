#pragma once

#include "ToolKit.h"

namespace ToolKit
{

  enum TransformationSpace
  {
    TS_WORLD,
    TS_PARENT,
    TS_LOCAL
  };

  class Node
  {
  public:
    Node();
    Node(glm::vec3 translation);
    void Translate(glm::vec3 val, TransformationSpace space = TS_PARENT);
    void Rotate(glm::quat val, TransformationSpace space = TS_PARENT);
    void Scale(glm::vec3 val);
    void Transform(glm::mat4 val, TransformationSpace space = TS_PARENT);
    glm::mat4 GetTransform(TransformationSpace space = TS_WORLD);
    glm::vec3 GetTranslation(TransformationSpace space = TS_PARENT);
    glm::quat GetOrientation(TransformationSpace space = TS_PARENT);
    glm::vec3 GetScale(TransformationSpace space = TS_PARENT);
    void AddChild(Node* child);

  public:
    glm::vec3 m_translation;
    glm::vec3 m_scale;
    glm::quat m_orientation;
    std::vector<Node*> m_children;
    Node* m_parent;
  };

}