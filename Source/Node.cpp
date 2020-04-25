#include "stdafx.h"
#include "Node.h"
#include "MathUtil.h"
#include "DebugNew.h"

namespace ToolKit
{

	Node::Node()
	{
	}

	void Node::Translate(const Vec3& val, TransformationSpace space)
	{
		Vec3 s;
		Quaternion q;
		Mat4 ts = glm::translate(glm::diagonal4x4(Vec4(1.0f)), val);
		TransformImp(ts, m_translation, q, s, space);
	}

	void Node::Rotate(const Quaternion& val, TransformationSpace space)
	{
		Vec3 t, s;
		TransformImp(glm::toMat4(val), t, m_orientation, s, space);
	}

	void Node::Scale(const Vec3& val, TransformationSpace space)
	{
		Vec3 t;
		Quaternion q;
		TransformImp(glm::diagonal4x4(Vec4(val, 1.0f)), t, q, m_scale, space);
	}

	void Node::Transform(const Mat4& val, TransformationSpace space)
	{
		TransformImp(val, m_translation, m_orientation, m_scale, space);
	}

	void Node::SetTransform(const Mat4& val, TransformationSpace space)
	{
		SetTransformImp(val, m_translation, m_orientation, m_scale, space);
	}

	Mat4 Node::GetTransform(TransformationSpace space) const
	{
		auto LocalTransform = [this]() -> Mat4
		{
			Mat4 scale;
			scale = glm::scale(scale, m_scale);
			Mat4 rotate;
			rotate = glm::toMat4(m_orientation);
			Mat4 translate;
			translate = glm::translate(translate, m_translation);
			return translate * rotate * scale;
		};

		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
				return ps * LocalTransform();
			}
			return LocalTransform();
		case TransformationSpace::TS_PARENT:
			return LocalTransform();
		case TransformationSpace::TS_LOCAL:
		default:
			return Mat4();
		}
	}

	void Node::SetTranslation(const Vec3& val, TransformationSpace space)
	{
		Vec3 s;
		Quaternion q;
		Mat4 ts = glm::translate(glm::diagonal4x4(Vec4(1.0f)), val);
		SetTransformImp(ts, m_translation, q, s, space);
	}

	Vec3 Node::GetTranslation(TransformationSpace space) const
	{
		Vec3 t, s;
		Quaternion q;
		GetTransformImp(t, q, s, space);
		return t;
	}

	void Node::SetOrientation(const Quaternion& val, TransformationSpace space)
	{
		Vec3 t, s;
		SetTransformImp(glm::toMat4(val), t, m_orientation, s, space);
	}

	Quaternion Node::GetOrientation(TransformationSpace space) const
	{
		Vec3 t, s;
		Quaternion q;
		GetTransformImp(t, q, s, space);
		return q;
	}

	// Fully tested.
	void Node::SetScale(const Vec3& val, TransformationSpace space)
	{
		Vec3 t;
		Quaternion q;
		SetTransformImp(glm::diagonal4x4(Vec4(val, 1.0f)), t, q, m_scale, space);
		return;

		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Mat3 ts, ps;
			ts = glm::diagonal3x3(val);
			Quaternion ws = GetOrientation(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ts = glm::inverse(ps) * ts * glm::toMat3(ws);
			Vec3 t;
			Quaternion q;
			DecomposeMatrix(ts, t, q, m_scale);
		}
		break;
		case TransformationSpace::TS_PARENT:
		{
			Mat3 ts, ps;
			ts = glm::diagonal3x3(val);
			Quaternion ws = GetOrientation(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ts = glm::inverse(ps) * ts * glm::inverse(ps) * glm::toMat3(ws);
			Vec3 t, s;
			Quaternion q;
			DecomposeMatrix(ts, t, q, m_scale);
		}
		break;
		case TransformationSpace::TS_LOCAL:
			m_scale = val;
		break;
		}
	}

	Vec3 Node::GetScale(TransformationSpace space) const
	{
		Vec3 t, s;
		Quaternion q;
		GetTransformImp(t, q, s, space);
		return s;
	}

	void Node::AddChild(Node* child)
	{
		m_children.push_back(child);
		child->m_parent = this;
	}

	void Node::TransformImp(const Mat4& val, Vec3& translation, Quaternion& orientation, Vec3& scale, TransformationSpace space)
	{
		Mat4 ps, ts, ws;
		if (m_parent != nullptr)
		{
			ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
		}

		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			ws = GetTransform(TransformationSpace::TS_WORLD);
			ts = glm::inverse(ps) * val * ws;
			break;
		case TransformationSpace::TS_PARENT:
			ws = GetTransform(TransformationSpace::TS_PARENT);
			ts = val * ws;
			break;
		case TransformationSpace::TS_LOCAL:
			ws = GetTransform(TransformationSpace::TS_WORLD);
			ts = glm::inverse(ps) * ws * val;
			break;
		}

		DecomposeMatrix(ts, translation, orientation, scale);
	}

	void Node::SetTransformImp(const Mat4& val, Vec3& translation, Quaternion& orientation, Vec3& scale, TransformationSpace space)
	{
		Mat4 ps, ts;
		if (m_parent != nullptr)
		{
			ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
		}

		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			ts = glm::inverse(ps) * val;
			break;
		case TransformationSpace::TS_PARENT:
			ts = val;
			break;
		case TransformationSpace::TS_LOCAL:
			Transform(val, TransformationSpace::TS_LOCAL);
			return;
		}

		DecomposeMatrix(ts, translation, orientation, scale);
	}

	void Node::GetTransformImp(Vec3& translation, Quaternion& orientation, Vec3& scale, TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ts = GetTransform(TransformationSpace::TS_WORLD);
				DecomposeMatrix(ts, translation, orientation, scale);
				break;
			} // Fall trough.
		case TransformationSpace::TS_PARENT:
			translation = m_translation;
			orientation = m_orientation;
			scale = m_scale;
			break;
		case TransformationSpace::TS_LOCAL:
		default:
			translation = Vec3();
			orientation = Quaternion();
			scale = Vec3(1.0f);
			break;
		}
	}

}
