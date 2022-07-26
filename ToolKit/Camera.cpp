#include "Camera.h"
#include "DirectionComponent.h"
#include "Node.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{
  Camera::Camera(XmlNode* node)
  {
    ParameterConstructor();

    DeSerialize(nullptr, node);

    DirectionComponentPtr dCom = GetComponent<DirectionComponent>();
    dCom->m_entity = this;

    if (IsOrtographic())
    {
      SetLens
      (
        GetLeftVal(),
        GetRightVal(),
        GetBottomVal(),
        GetTopVal(),
        GetNearVal(),
        GetFarVal()
      );
    }
    else
    {
      SetLens(GetFovVal(), GetAspectVal() * GetHeightVal(), GetHeightVal());
    }
  }

  Camera::Camera()
  {
    ParameterConstructor();

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

    SetFovVal(fov);
    SetAspectVal(width / height);
    SetNearVal(near);
    SetFarVal(far);
    SetHeightVal(height);
    SetOrthographicVal(false);
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

    SetLeftVal(left);
    SetRightVal(right);
    SetTopVal(top);
    SetBottomVal(bottom);
    SetFovVal(0.0f);
    SetAspectVal(1.0f);
    SetNearVal(near);
    SetFarVal(far);
    SetHeightVal(top - bottom);
    SetOrthographicVal(true);
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
    return GetOrthographicVal();
  }

  Camera::CamData Camera::GetData() const
  {
    CamData data;
    DirectionComponentPtr dcp = GetComponent<DirectionComponent>();
    assert(dcp);
    data.dir = dcp->GetDirection();

    data.pos = m_node->GetTranslation(TransformationSpace::TS_WORLD);
    data.projection = m_projection;
    data.fov = GetFovVal();
    data.aspect = GetAspectVal();
    data.nearDist = GetNearVal();
    data.height = GetHeightVal();
    data.ortographic = GetOrthographicVal();

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
  }

  void Camera::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    ClearComponents();
    Entity::DeSerialize(doc, parent);
  }

  Entity* Camera::CopyTo(Entity* copyTo) const
  {
    WeakCopy(copyTo, false);
    Camera* cpy = static_cast<Camera*> (copyTo);
    cpy->m_projection = m_projection;
    cpy->ClearComponents();
    cpy->AddComponent(new DirectionComponent(cpy));

    return cpy;
  }

  void Camera::ParameterConstructor()
  {
    Orthographic_Define
    (
      false,
      "Camera",
      90,
      true,
      false
    );

    Fov_Define
    (
      glm::radians(90.0f),
      "Camera",
      90,
      true,
      true
    );

    Aspect_Define
    (
      640.0f / 480.0f,
      "Camera",
      90,
      true,
      true
    );

    Near_Define
    (
      0.5f,
      "Camera",
      90,
      true,
      true
    );

    Far_Define
    (
      1000.0f,
      "Camera",
      90,
      true,
      true
    );

    Height_Define
    (
      480,
      "Camera",
      90,
      !IsOrtographic(),
      true
    );

    Left_Define
    (
      10.0f,
      "Camera",
      90,
      IsOrtographic(),
      true
    );

    Right_Define
    (
      -10.0f,
      "Camera",
      90,
      IsOrtographic(),
      true
    );

    Bottom_Define
    (
      -10.0f,
      "Camera",
      90,
      IsOrtographic(),
      true
    );

    Top_Define
    (
      10.0f,
      "Camera",
      90,
      IsOrtographic(),
      true
    );
  }

}  // namespace ToolKit
