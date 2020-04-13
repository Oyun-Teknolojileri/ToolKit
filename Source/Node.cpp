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

	Node::Node(Vec3 translation)
	{
		m_parent = nullptr;
		m_scale = Vec3(1, 1, 1);
		m_translation = translation;
	}

	void Node::Translate(Vec3 val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Mat4 ts, ps, ws;
			ts = glm::translate(ts, val);
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
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
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			m_translation = glm::column(glm::inverse(ps) * ws * ts, 3).xyz;
		}
		break;
		}
	}

	void Node::Rotate(Quaternion val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Quaternion ps, ws;
			ws = GetOrientation(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
				ps = m_parent->GetOrientation(TransformationSpace::TS_WORLD);
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
				ps = m_parent->GetOrientation(TransformationSpace::TS_WORLD);
			m_orientation = glm::inverse(ps) * ws * val;
		}
		}

		m_orientation = glm::normalize(m_orientation);
	}

	void Node::Scale(Vec3 val)
	{
		m_scale += val;
	}

	void Node::Transform(Mat4 val, TransformationSpace space)
	{
		Vec3 translation;
		Quaternion orientation;
		DecomposeMatrix(val, translation, orientation);

		Translate(translation, space);
		Rotate(orientation, space);
	}

	Mat4 Node::GetTransform(TransformationSpace space)
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
				return m_parent->GetTransform(TransformationSpace::TS_WORLD) * constructTransform();
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

	Vec3 Node::GetTranslation(TransformationSpace space)
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

	Quaternion Node::GetOrientation(TransformationSpace space)
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

	Vec3 Node::GetScale(TransformationSpace space)
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

}
