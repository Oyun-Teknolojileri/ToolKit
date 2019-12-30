#include "stdafx.h"
#include "Node.h"
#include "MathUtil.h"

ToolKit::Node::Node()
{
  m_parent = nullptr;
  m_scale = glm::vec3(1, 1, 1);
}

ToolKit::Node::Node(glm::vec3 translation)
{
  m_parent = nullptr;
  m_scale = glm::vec3(1, 1, 1);
  m_translation = translation;
}

void ToolKit::Node::Translate(glm::vec3 val, TransformationSpace space)
{
  switch (space)
  {
  case ToolKit::TS_WORLD:
  {
    glm::mat4 ts, ps, ws;
    ts = glm::translate(ts, val);
    ws = GetTransform(ToolKit::TS_WORLD);
    if (m_parent != nullptr)
      ps = m_parent->GetTransform(ToolKit::TS_WORLD);
    m_translation = glm::column(glm::inverse(ps) * ts * ws, 3).xyz;
  }
  break;
  case ToolKit::TS_PARENT:
  {
    m_translation = val + m_translation;
  }
  break;
  case ToolKit::TS_LOCAL:
  {
    glm::mat4 ts, ps, ws;
    ts = glm::translate(ts, val);
    ws = GetTransform(ToolKit::TS_WORLD);
    if (m_parent != nullptr)
      ps = m_parent->GetTransform(ToolKit::TS_WORLD);
    m_translation = glm::column(glm::inverse(ps) * ws * ts, 3).xyz;
  }
  break;
  }
}

void ToolKit::Node::Rotate(glm::quat val, TransformationSpace space)
{
  switch (space)
  {
  case ToolKit::TS_WORLD:
  {
    glm::quat ps, ws;
    ws = GetOrientation(ToolKit::TS_WORLD);
    if (m_parent != nullptr)
      ps = m_parent->GetOrientation(ToolKit::TS_WORLD);
    m_orientation = glm::inverse(ps) * val * ws;
  }
  break;
  case ToolKit::TS_PARENT:
    m_orientation = val * m_orientation;
    break;
  case ToolKit::TS_LOCAL:
  {
    glm::quat ps, ws;
    ws = GetOrientation(ToolKit::TS_WORLD);
    if (m_parent != nullptr)
      ps = m_parent->GetOrientation(ToolKit::TS_WORLD);
    m_orientation = glm::inverse(ps) * ws * val;
  }
  }

  m_orientation = glm::normalize(m_orientation);
}

void ToolKit::Node::Scale(glm::vec3 val)
{
  m_scale += val;
}

void ToolKit::Node::Transform(glm::mat4 val, TransformationSpace space)
{
  glm::vec3 translation;
  glm::quat orientation;
  DecomposeMatrix(val, translation, orientation);

  Translate(translation, space);
  Rotate(orientation, space);
}

glm::mat4 ToolKit::Node::GetTransform(TransformationSpace space)
{
  auto constructTransform = [this]() -> glm::mat4
  {
    glm::mat4 scale;
    scale = glm::scale(scale, m_scale);
    glm::mat4 rotate;
    rotate = glm::toMat4(m_orientation);
    glm::mat4 translate;
    translate = glm::translate(translate, m_translation);
    return translate * rotate * scale;
  };

  switch (space)
  {
  case ToolKit::TS_WORLD:
    if (m_parent != nullptr)
      return m_parent->GetTransform(ToolKit::TS_WORLD) * constructTransform();
    return constructTransform();
    break;
  case ToolKit::TS_PARENT:
    return constructTransform();
    break;
  case ToolKit::TS_LOCAL:
  default:
    return glm::mat4();
  }
}

glm::vec3 ToolKit::Node::GetTranslation(TransformationSpace space)
{
  switch (space)
  {
  case ToolKit::TS_WORLD:
    if (m_parent != nullptr)
      return (m_parent->GetTransform(ToolKit::TS_WORLD) * glm::vec4(m_translation, 1.0f)).xyz;
    return m_translation;
    break;
  case ToolKit::TS_PARENT:
    return m_translation;
    break;
  case ToolKit::TS_LOCAL:
  default:
    return glm::vec3();
    break;
  }
}

glm::quat ToolKit::Node::GetOrientation(TransformationSpace space)
{
  switch (space)
  {
  case ToolKit::TS_WORLD:
    if (m_parent != nullptr)
      return glm::normalize(m_parent->GetOrientation(ToolKit::TS_WORLD) * m_orientation);
    return m_orientation;
    break;
  case ToolKit::TS_PARENT:
    return m_orientation;
    break;
  case ToolKit::TS_LOCAL:
  default:
    return glm::quat();
    break;
  }
}

glm::vec3 ToolKit::Node::GetScale(TransformationSpace space)
{
  switch (space)
  {
  case ToolKit::TS_WORLD:
    if (m_parent != nullptr)
      return m_parent->GetScale(ToolKit::TS_WORLD) + m_scale;
    return m_scale;
    break;
  case ToolKit::TS_PARENT:
    return m_scale;
    break;
  case ToolKit::TS_LOCAL:
  default:
    return glm::vec3();
    break;
  }
}

void ToolKit::Node::AddChild(Node* child)
{
  m_children.push_back(child);
  child->m_parent = this;
}
