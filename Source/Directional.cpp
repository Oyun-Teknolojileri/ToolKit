#include "stdafx.h"
#include "Directional.h"
#include "Node.h"
#include "DebugNew.h"

ToolKit::Directional::Directional()
{
}

ToolKit::Directional::~Directional()
{
}

void ToolKit::Directional::Pitch(float val)
{
  glm::quat q = glm::angleAxis(val, glm::vec3(1.0f, 0.0f, 0.0f));
  m_node->Rotate(q, TransformationSpace::TS_LOCAL);
}

void ToolKit::Directional::Yaw(float val)
{
  glm::quat q = glm::angleAxis(val, glm::vec3(0.0f, 1.0f, 0.0f));
  m_node->Rotate(q, TransformationSpace::TS_LOCAL);
}

void ToolKit::Directional::Roll(float val)
{
  glm::quat q = glm::angleAxis(val, glm::vec3(1.0f, 0.0f, 0.0f));
  m_node->Rotate(q, TransformationSpace::TS_LOCAL);
}

void ToolKit::Directional::Translate(glm::vec3 pos)
{
  m_node->Translate(pos, TransformationSpace::TS_LOCAL);
}

void ToolKit::Directional::RotateOnUpVector(float val)
{
  m_node->Rotate(glm::angleAxis(val, glm::vec3(0.0f, 1.0f, 0.0f)), TransformationSpace::TS_WORLD);
}

void ToolKit::Directional::GetLocalAxis(glm::vec3& dir, glm::vec3& up, glm::vec3& right)
{
  glm::mat4 transform = m_node->GetTransform();
  right = glm::column(transform, 0);
  up = glm::column(transform, 1);
  dir = -glm::column(transform, 2);
}

ToolKit::EntityType ToolKit::Directional::GetType()
{
  return EntityType::Entity_Directional;
}

ToolKit::Camera::Camera()
{
  SetLens(glm::radians(90.0f), 640.0f, 480.0f, 0.01f, 1000.0f);
}

ToolKit::Camera::~Camera()
{
}

void ToolKit::Camera::SetLens(float fov, float width, float height)
{
	SetLens(fov, width, height, 0.01f, 1000.0f);
}

void ToolKit::Camera::SetLens(float fov, float width, float height, float near, float far)
{
  m_projection = glm::perspectiveFov(fov, width, height, near, far);
  m_fov = fov;
  m_aspect = width / height;
  m_near = near;
  m_height = height;
  m_ortographic = false;
}

void ToolKit::Camera::SetLens(float aspect, float left, float right, float bottom, float top, float near, float far)
{
  m_projection = glm::ortho(left * aspect, right * aspect, bottom, top, near, far);
	m_fov = 0.0f;
  m_aspect = aspect;
  m_near = near;
  m_height = top - bottom;
	m_ortographic = true;
}

glm::mat4 ToolKit::Camera::GetViewMatrix()
{
  glm::mat4 view = m_node->GetTransform(TransformationSpace::TS_WORLD);
  return glm::inverse(view);
}

ToolKit::Camera::CamData ToolKit::Camera::GetData()
{
  CamData data;
  glm::vec3 tmp;
  GetLocalAxis(data.dir, tmp, tmp);
  data.pos = m_node->GetTranslation(TransformationSpace::TS_WORLD);
  data.projection = m_projection;
  data.fov = m_fov;
  data.aspect = m_aspect;
  data.nearDist = m_near;
  data.height = m_height;
  data.ortographic = m_ortographic;

  return data;
}

ToolKit::EntityType ToolKit::Camera::GetType()
{
  return EntityType::Entity_Camera;
}

ToolKit::Light::Light()
{
  m_color = glm::vec3(1.0f, 1.0f, 1.0f);
}

ToolKit::Light::~Light()
{
}

ToolKit::Light::LightData ToolKit::Light::GetData()
{
  LightData data;
  GetLocalAxis(data.dir, data.pos, data.color);
  data.pos = m_node->GetTranslation(TransformationSpace::TS_WORLD);
  data.color = m_color;

  return data;
}

ToolKit::EntityType ToolKit::Light::GetType()
{
  return EntityType::Entity_Light;
}
