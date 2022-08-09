#include "Camera.h"
#include "DirectionComponent.h"
#include "Node.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  Camera::Camera(XmlNode* node)
  {
    DeSerialize(nullptr, node);

    DirectionComponentPtr dCom = GetComponent<DirectionComponent>();
    dCom->m_entity = this;

    if (m_ortographic)
    {
      SetLens(m_left, m_right, m_bottom, m_top, m_near, m_far);
    }
    else
    {
      SetLens(m_fov, m_aspect * m_height, m_height);
    }
  }

  Camera::Camera()
  {
    SetLens(glm::radians(90.0f), 640.0f, 480.0f, 0.01f, 1000.0f);
    AddComponent(new DirectionComponent(this));
  }

  Camera::~Camera()
  {
  }

  void Camera::SetLens(float fov, float width, float height)
  {
    SetLens(fov, width, height, 0.5f, 1000.0f);
  }

  void Camera::SetLens
  (
    float fov,
    float width,
    float height,
    float near,
    float far
  )
  {
    m_projection = glm::perspectiveFov(fov, width, height, near, far);
    m_fov = fov;
    m_aspect = width / height;
    m_near = near;
    m_far = far;
    m_height = height;
    m_ortographic = false;
  }

  void Camera::SetLens
  (
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far
  )
  {
    m_projection = glm::ortho(left, right, bottom, top, near, far);
    m_left = left;
    m_right = right;
    m_top = top;
    m_bottom = bottom;
    m_fov = 0.0f;
    m_aspect = 1.0f;
    m_near = near;
    m_far = far;
    m_height = top - bottom;
    m_ortographic = true;
  }

  Mat4 Camera::GetViewMatrix() const
  {
    Mat4 view = m_node->GetTransform(TransformationSpace::TS_WORLD);
    return glm::inverse(view);
  }

  Mat4 Camera::GetProjectionMatrix() const
  {
    return m_projection;
  }

  void Camera::SetProjectionMatrix(Mat4 proj)
  {
    m_projection = proj;
  }

  bool Camera::IsOrtographic() const
  {
    return m_ortographic;
  }

  Camera::CamData Camera::GetData() const
  {
    CamData data;
    DirectionComponentPtr dcp = GetComponent<DirectionComponent>();
    assert(dcp);
    data.dir = dcp->GetDirection();

    data.pos = m_node->GetTranslation(TransformationSpace::TS_WORLD);
    data.projection = m_projection;
    data.fov = m_fov;
    data.aspect = m_aspect;
    data.nearDist = m_near;
    data.height = m_height;
    data.ortographic = m_ortographic;

    return data;
  }

  EntityType Camera::GetType() const
  {
    return EntityType::Entity_Camera;
  }

  void Camera::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    parent = parent->last_node();

    XmlNode* node = CreateXmlNode(doc, "Camera", parent);

    WriteAttr(node, doc, "fov", std::to_string(m_fov));
    WriteAttr(node, doc, "aspect", std::to_string(m_aspect));
    WriteAttr(node, doc, "near", std::to_string(m_near));
    WriteAttr(node, doc, "far", std::to_string(m_far));
    WriteAttr(node, doc, "height", std::to_string(m_height));
    WriteAttr(node, doc, "ortographic", std::to_string(m_ortographic));
    WriteAttr(node, doc, "left", std::to_string(m_left));
    WriteAttr(node, doc, "right", std::to_string(m_right));
    WriteAttr(node, doc, "top", std::to_string(m_top));
    WriteAttr(node, doc, "bottom", std::to_string(m_bottom));
  }

  void Camera::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents();
    Entity::DeSerialize(doc, parent);
    if (XmlNode* node = parent->first_node("Camera"))
    {
      ReadAttr(node, "fov", m_fov);
      ReadAttr(node, "aspect", m_aspect);
      ReadAttr(node, "near", m_near);
      ReadAttr(node, "far", m_far);
      ReadAttr(node, "height", m_height);
      ReadAttr(node, "ortographic", m_ortographic);
      ReadAttr(node, "left", m_left);
      ReadAttr(node, "right", m_right);
      ReadAttr(node, "top", m_top);
      ReadAttr(node, "bottom", m_bottom);
    }
  }

  Entity* Camera::CopyTo(Entity* copyTo) const
  {
    WeakCopy(copyTo, false);
    Camera* cpy = static_cast<Camera*> (copyTo);
    cpy->m_fov = m_fov;
    cpy->m_aspect = m_aspect;
    cpy->m_near = m_near;
    cpy->m_far = m_far;
    cpy->m_height = m_height;
    cpy->m_left = m_left;
    cpy->m_right = m_right;
    cpy->m_bottom = m_bottom;
    cpy->m_ortographic = m_ortographic;
    cpy->m_projection = m_projection;
    cpy->ClearComponents();
    cpy->AddComponent(new DirectionComponent(cpy));

    return cpy;
  }

}  // namespace ToolKit
