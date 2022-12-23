#include "Camera.h"

#include "DirectionComponent.h"
#include "Node.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  Camera::Camera()
  {
    SetLens(glm::radians(90.0f), 640.0f / 480.0f, 0.01f, 1000.0f);
    AddComponent(new DirectionComponent(this));

    ParameterConstructor();
    ParameterEventConstructor();
  }

  Camera::~Camera() {}

  void Camera::SetLens(float fov, float aspect)
  {
    SetLens(fov, aspect, 0.5f, 1000.0f);
  }

  void Camera::SetLens(float fov, float aspect, float near, float far)
  {
    m_projection  = glm::perspectiveFov(fov, aspect, 1.0f, near, far);
    m_fov         = fov;
    m_aspect      = aspect;
    m_near        = near;
    m_far         = far;
    m_ortographic = false;
  }

  void Camera::SetLens(float left,
                       float right,
                       float bottom,
                       float top,
                       float near,
                       float far)
  {
    m_aspect      = (right - left) / (top - bottom);
    m_projection  = glm::ortho(left * m_orthographicScale,
                              right * m_orthographicScale,
                              bottom * m_orthographicScale,
                              top * m_orthographicScale,
                              near,
                              far);

    m_left        = left;
    m_right       = right;
    m_top         = top;
    m_bottom      = bottom;
    m_near        = near;
    m_far         = far;
    m_ortographic = true;
  }

  Mat4 Camera::GetViewMatrix() const
  {
    Mat4 view = m_node->GetTransform();
    return glm::inverse(view);
  }

  Mat4 Camera::GetProjectionMatrix() const { return m_projection; }

  bool Camera::IsOrtographic() const { return m_ortographic; }

  Camera::CamData Camera::GetData() const
  {
    CamData data;
    DirectionComponentPtr dcp = GetComponent<DirectionComponent>();
    assert(dcp);
    data.dir         = dcp->GetDirection();

    data.pos         = m_node->GetTranslation();
    data.projection  = m_projection;
    data.fov         = m_fov;
    data.aspect      = m_aspect;
    data.nearDist    = m_near;
    data.far         = m_far;
    data.ortographic = m_ortographic;

    return data;
  }

  EntityType Camera::GetType() const { return EntityType::Entity_Camera; }

  void Camera::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    parent        = parent->last_node();

    XmlNode* node = CreateXmlNode(doc, "Camera", parent);

    WriteAttr(node, doc, "fov", std::to_string(m_fov));
    WriteAttr(node, doc, "aspect", std::to_string(m_aspect));
    WriteAttr(node, doc, "near", std::to_string(m_near));
    WriteAttr(node, doc, "far", std::to_string(m_far));
    WriteAttr(node, doc, "ortographic", std::to_string(m_ortographic));
    WriteAttr(node, doc, "left", std::to_string(m_left));
    WriteAttr(node, doc, "right", std::to_string(m_right));
    WriteAttr(node, doc, "top", std::to_string(m_top));
    WriteAttr(node, doc, "bottom", std::to_string(m_bottom));
    WriteAttr(node, doc, "scale", std::to_string(m_orthographicScale));
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
      ReadAttr(node, "ortographic", m_ortographic);
      ReadAttr(node, "left", m_left);
      ReadAttr(node, "right", m_right);
      ReadAttr(node, "top", m_top);
      ReadAttr(node, "bottom", m_bottom);
      ReadAttr(node, "scale", m_orthographicScale);
    }

    if (m_ortographic)
    {
      SetLens(m_left, m_right, m_bottom, m_top, m_near, m_far);
    }
    else
    {
      SetLens(m_fov, m_aspect, m_near, m_far);
    }
  }

  // https://stackoverflow.com/questions/2866350/move-camera-to-fit-3d-scene
  void Camera::FocusToBoundingBox(const BoundingBox& bb, float margin)
  {
    if (m_ortographic)
    {
      return;
    }

    Vec3 geoCenter = (bb.max + bb.min) * 0.5f;
    float r        = glm::distance(geoCenter, bb.max) * margin;
    float d        = r / glm::tan(m_fov / 2.0f);
    Vec3 eye       = geoCenter + glm::normalize(Vec3(1.0f)) * d;
    m_node->SetTranslation(eye, TransformationSpace::TS_WORLD);
    GetComponent<DirectionComponent>()->LookAt(geoCenter);
  }

  float Camera::Fov() const { return m_fov; }

  float Camera::Aspect() const { return m_aspect; }

  float Camera::Near() const { return m_near; }

  float Camera::Far() const { return m_far; }

  float Camera::Left() const { return m_left; }

  float Camera::Right() const { return m_right; }

  float Camera::Top() const { return m_top; }

  float Camera::Bottom() const { return m_bottom; }

  Vec3 Camera::Position() const { return m_node->GetTranslation(); }

  Vec3 Camera::Direction() const
  {
    DirectionComponentPtr dcp = GetComponent<DirectionComponent>();
    return dcp->GetDirection();
  }

  Entity* Camera::CopyTo(Entity* copyTo) const
  {
    WeakCopy(copyTo, false);
    Camera* cpy              = static_cast<Camera*>(copyTo);
    cpy->m_fov               = m_fov;
    cpy->m_aspect            = m_aspect;
    cpy->m_near              = m_near;
    cpy->m_far               = m_far;
    cpy->m_left              = m_left;
    cpy->m_right             = m_right;
    cpy->m_bottom            = m_bottom;
    cpy->m_ortographic       = m_ortographic;
    cpy->m_projection        = m_projection;
    cpy->m_orthographicScale = m_orthographicScale;
    cpy->ClearComponents();
    cpy->AddComponent(new DirectionComponent(cpy));

    return cpy;
  }

  void Camera::ParameterConstructor()
  {
    Fov_Define(glm::degrees(m_fov),
               CameraCategory.Name,
               CameraCategory.Priority,
               true,
               true,
               {false, true, 10.0f, 175.0f, 5.0f});

    NearClip_Define(m_near,
                    CameraCategory.Name,
                    CameraCategory.Priority,
                    true,
                    true,
                    {false, true, 0.1f, 100.0f, 0.1f});

    FarClip_Define(m_far,
                   CameraCategory.Name,
                   CameraCategory.Priority,
                   true,
                   true,
                   {false, true, 100.1f, 5000.0f, 10.0f});

    Orthographic_Define(m_ortographic,
                        CameraCategory.Name,
                        CameraCategory.Priority,
                        true,
                        true);

    OrthographicScale_Define(m_orthographicScale,
                             CameraCategory.Name,
                             CameraCategory.Priority,
                             true,
                             true,
                             {false, true, 0.001f, 100.0f, 0.001f});
  }

  void Camera::ParameterEventConstructor()
  {
    auto updateLensInternalFn = [this]()
    {
      if (m_ortographic)
      {
        SetLens(Left(), Right(), Top(), Bottom(), Near(), Far());
      }
      else
      {
        SetLens(Fov(), Aspect(), Near(), Far());
      }
    };

    ParamFov().m_onValueChangedFn.push_back(
        [this, updateLensInternalFn](Value& oldVal, Value& newVal) -> void
        {
          float degree = std::get<float>(newVal);
          m_fov        = glm::radians(degree);
          updateLensInternalFn();
        });

    ParamNearClip().m_onValueChangedFn.push_back(
        [this, updateLensInternalFn](Value& oldVal, Value& newVal) -> void
        {
          m_near = std::get<float>(newVal);
          updateLensInternalFn();
        });

    ParamFarClip().m_onValueChangedFn.push_back(
        [this, updateLensInternalFn](Value& oldVal, Value& newVal) -> void
        {
          m_far = std::get<float>(newVal);
          updateLensInternalFn();
        });

    ParamOrthographic().m_onValueChangedFn.push_back(
        [this, updateLensInternalFn](Value& oldVal, Value& newVal) -> void
        {
          m_ortographic = std::get<bool>(newVal);
          updateLensInternalFn();
        });

    ParamOrthographicScale().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void
        {
          m_orthographicScale = std::get<float>(newVal);

          if (m_ortographic)
          {
            SetLens(Left(), Right(), Top(), Bottom(), Near(), Far());
          }
        });
  }

} // namespace ToolKit
