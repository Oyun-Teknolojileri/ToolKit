#include "stdafx.h"
#include "Node.h"
#include "MathUtil.h"
#include "DebugNew.h"

namespace ToolKit
{

	Node::Node()
	{
		m_parent = nullptr;
		m_scale = Vec3(1, 1, 1);
	}

	Node::Node(const Vec3& translation)
	{
		m_parent = nullptr;
		m_scale = Vec3(1, 1, 1);
		m_translation = translation;
	}

	void Node::Translate(const Vec3& val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Mat4 ts, ps, ws;
			ts = glm::translate(ts, val);
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			m_translation = glm::column(glm::inverse(ps) * ts * ws, 3).xyz;
		}
		break;
		case TransformationSpace::TS_PARENT:
		{
			m_translation = val + m_translation;
		}
		break;
		case TransformationSpace::TS_LOCAL:
		{
			Mat4 ts, ps, ws;
			ts = glm::translate(ts, val);
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			m_translation = glm::column(glm::inverse(ps) * ws * ts, 3).xyz;
		}
		break;
		}
	}

	void Node::Rotate(const Quaternion& val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Quaternion ps, ws;
			ws = GetOrientation(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetOrientation(TransformationSpace::TS_WORLD);
			}
			m_orientation = glm::inverse(ps) * val * ws;
		}
		break;
		case TransformationSpace::TS_PARENT:
			m_orientation = val * m_orientation;
			break;
		case TransformationSpace::TS_LOCAL:
		{
			Quaternion ps, ws;
			ws = GetOrientation(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetOrientation(TransformationSpace::TS_WORLD);
			}
			m_orientation = glm::inverse(ps) * ws * val;
		}
		}

		m_orientation = glm::normalize(m_orientation);
	}

	void Node::Scale(const Vec3& val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Vec3 ps, ws;
			ws = GetScale(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetScale(TransformationSpace::TS_WORLD);
			}
			m_scale = (1.0f / ps) * val * ws;
		}
		break;
		case TransformationSpace::TS_PARENT:
			m_scale = val * m_scale;
			break;
		case TransformationSpace::TS_LOCAL:
		{
			Vec3 ps, ws;
			ws = GetScale(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetScale(TransformationSpace::TS_WORLD);
			}
				
			m_scale = (1.0f / ps) * ws * val;
		}
		}
	}

	void Node::Transform(const Mat4& val, TransformationSpace space)
	{
		Vec3 translation;
		Vec3 scale;
		Quaternion orientation;
		DecomposeMatrix(val, translation, orientation, scale);

		Translate(translation, space);
		Rotate(orientation, space);
		m_scale = scale;
	}

	void Node::SetTransform(const Mat4& val, TransformationSpace space)
	{

	}

	Mat4 Node::GetTransform(TransformationSpace space) const
	{
		auto constructTransform = [this]() -> Mat4
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
				return m_parent->GetTransform(TransformationSpace::TS_WORLD) * constructTransform();
			}
			return constructTransform();
			break;
		case TransformationSpace::TS_PARENT:
			return constructTransform();
			break;
		case TransformationSpace::TS_LOCAL:
		default:
			return Mat4();
		}
	}

	void Node::SetTranslation(const Vec3& val, TransformationSpace space)
	{
		Mat4 ts;
		ts = glm::translate(ts, val);

		GetOffsetToParentSpace(ts, space);

		// Extract, apply offset.
		m_translation = glm::column(ts, 3).xyz;
	}

	Vec3 Node::GetTranslation(TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
				return (m_parent->GetTransform(TransformationSpace::TS_WORLD) * Vec4(m_translation, 1.0f)).xyz;
			return m_translation;
			break;
		case TransformationSpace::TS_PARENT:
			return m_translation;
			break;
		case TransformationSpace::TS_LOCAL:
		default:
			return Vec3();
			break;
		}
	}

	void Node::SetOrientation(const Quaternion& val, TransformationSpace space)
	{
		Mat4 ts = glm::toMat4(val);

		GetOffsetToParentSpace(ts, space);

		// Extract, apply offset.
		m_orientation = glm::normalize(glm::toQuat(ts));
	}

	Quaternion Node::GetOrientation(TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
				return glm::normalize(m_parent->GetOrientation(TransformationSpace::TS_WORLD) * m_orientation);
			return m_orientation;
			break;
		case TransformationSpace::TS_PARENT:
			return m_orientation;
			break;
		case TransformationSpace::TS_LOCAL:
		default:
			return Quaternion();
			break;
		}
	}

	void Node::SetScale(const Vec3& val, TransformationSpace space)
	{
		Mat4 ts;
		ts = glm::translate(ts, val);

		GetOffsetToParentSpace(ts, space);

		Vec3 t, s;
		Quaternion q;
		DecomposeMatrix(ts, t, q, s);

		// Extract, apply offset.
		m_scale = s;
	}

	Vec3 Node::GetScale(TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
				return m_parent->GetScale(TransformationSpace::TS_WORLD) + m_scale;
			return m_scale;
			break;
		case TransformationSpace::TS_PARENT:
			return m_scale;
			break;
		case TransformationSpace::TS_LOCAL:
		default:
			return Vec3();
			break;
		}
	}

	void Node::AddChild(Node* child)
	{
		m_children.push_back(child);
		child->m_parent = this;
	}

	void Node::GetOffsetToParentSpace(Mat4& val, TransformationSpace space) const
	{
		// Transform in queried space.
		Mat4 itsQuerySpace = GetTransform(TransformationSpace::TS_WORLD);
		itsQuerySpace = glm::inverse(itsQuerySpace);
		val = itsQuerySpace * val; 

		// Offset in parent space.
		Mat4 itsParentSpace = GetTransform(TransformationSpace::TS_PARENT);
		itsParentSpace = glm::inverse(itsParentSpace);
		val = itsParentSpace * val;
	}

}
