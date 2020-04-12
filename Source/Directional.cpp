#include "stdafx.h"
#include "Directional.h"
#include "Node.h"
#include "DebugNew.h"

namespace ToolKit
{

	Directional::Directional()
	{
	}

	Directional::~Directional()
	{
	}

	void Directional::Pitch(float val)
	{
		glm::quat q = glm::angleAxis(val, glm::vec3(1.0f, 0.0f, 0.0f));
		m_node->Rotate(q, TransformationSpace::TS_LOCAL);
	}

	void Directional::Yaw(float val)
	{
		glm::quat q = glm::angleAxis(val, glm::vec3(0.0f, 1.0f, 0.0f));
		m_node->Rotate(q, TransformationSpace::TS_LOCAL);
	}

	void Directional::Roll(float val)
	{
		glm::quat q = glm::angleAxis(val, glm::vec3(1.0f, 0.0f, 0.0f));
		m_node->Rotate(q, TransformationSpace::TS_LOCAL);
	}

	void Directional::Translate(glm::vec3 pos)
	{
		m_node->Translate(pos, TransformationSpace::TS_LOCAL);
	}

	void Directional::RotateOnUpVector(float val)
	{
		m_node->Rotate(glm::angleAxis(val, glm::vec3(0.0f, 1.0f, 0.0f)), TransformationSpace::TS_WORLD);
	}

	void Directional::GetLocalAxis(glm::vec3& dir, glm::vec3& up, glm::vec3& right) const
	{
		glm::mat4 transform = m_node->GetTransform();
		right = glm::column(transform, 0);
		up = glm::column(transform, 1);
		dir = -glm::column(transform, 2);
	}

	glm::vec3 Directional::GetDir() const
	{
		glm::mat4 transform = m_node->GetTransform();
		return -glm::column(transform, 2);
	}

	glm::vec3 Directional::GetUp() const
	{
		glm::mat4 transform = m_node->GetTransform();
		return glm::column(transform, 1);
	}

	glm::vec3 Directional::GetRight() const
	{
		glm::mat4 transform = m_node->GetTransform();
		return glm::column(transform, 0);
	}

	EntityType Directional::GetType() const
	{
		return EntityType::Entity_Directional;
	}

	Camera::Camera()
	{
		SetLens(glm::radians(90.0f), 640.0f, 480.0f, 0.01f, 1000.0f);
	}

	Camera::~Camera()
	{
	}

	void Camera::SetLens(float fov, float width, float height)
	{
		SetLens(fov, width, height, 0.01f, 1000.0f);
	}

	void Camera::SetLens(float fov, float width, float height, float near, float far)
	{
		m_projection = glm::perspectiveFov(fov, width, height, near, far);
		m_fov = fov;
		m_aspect = width / height;
		m_near = near;
		m_height = height;
		m_ortographic = false;
	}

	void Camera::SetLens(float aspect, float left, float right, float bottom, float top, float near, float far)
	{
		m_projection = glm::ortho(left * aspect, right * aspect, bottom, top, near, far);
		m_fov = 0.0f;
		m_aspect = aspect;
		m_near = near;
		m_height = top - bottom;
		m_ortographic = true;
	}

	glm::mat4 Camera::GetViewMatrix() const
	{
		glm::mat4 view = m_node->GetTransform(TransformationSpace::TS_WORLD);
		return glm::inverse(view);
	}

	Camera::CamData Camera::GetData() const
	{
		CamData data;
		data.dir = GetDir();
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

	Light::Light()
	{
		m_color = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	Light::~Light()
	{
	}

	Light::LightData Light::GetData() const
	{
		LightData data;
		data.dir = GetDir();
		data.pos = m_node->GetTranslation(TransformationSpace::TS_WORLD);
		data.color = m_color;

		return data;
	}

	EntityType Light::GetType() const
	{
		return EntityType::Entity_Light;
	}

}
