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
			ts = glm::inverse(ps) * ts * ws;
			m_translation = ts[3].xyz;
		}
		break;
		case TransformationSpace::TS_PARENT:
			m_translation = val + m_translation;
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
			ts = glm::inverse(ps) * ws * ts;
			m_translation = ts[3].xyz;
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
			Mat3 ps, ws;
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			Mat3 ts = glm::inverse(ps) * glm::toMat3(val) * ws;
			m_orientation = glm::toQuat(ts);
		}
		break;
		case TransformationSpace::TS_PARENT:
			m_orientation = val * m_orientation;
			break;
		case TransformationSpace::TS_LOCAL:
		{
			Mat3 ps, ws;
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			Mat3 ts = glm::inverse(ps) * ws * glm::toMat3(val);
			m_orientation = glm::toQuat(ts);
		}
		break;
		}

		m_orientation = glm::normalize(m_orientation);
	}

	void Node::Scale(const Vec3& val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Mat3 ts, ps, ws;
			ts = glm::diagonal3x3(val);
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ts = glm::inverse(ps) * ts * ws;
			Vec3 t;
			Quaternion q;
			DecomposeMatrix(ts, t, q, m_scale);
		}
		break;
		case TransformationSpace::TS_PARENT:
		{
			Mat3 ts, ps, ws;
			ts = glm::diagonal3x3(val);
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ts = glm::inverse(ps) * ts * glm::inverse(ps) * ws;
			Vec3 t, s;
			Quaternion q;
			DecomposeMatrix(ts, t, q, m_scale);
		}
		break;
		case TransformationSpace::TS_LOCAL:
		{
			Mat3 ts, ps, ws;
			ts = glm::diagonal3x3(val);
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ts = glm::inverse(ps) * ws * ts;
			Vec3 t;
			Quaternion q;
			DecomposeMatrix(ts, t, q, m_scale);
		}
		break;
		}
	}

	void Node::Transform(const Mat4& val, TransformationSpace space)
	{
		Mat4 ips, ws;
		ws = GetTransform(TransformationSpace::TS_WORLD);
		if (m_parent != nullptr)
		{
			ips = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			ips = glm::inverse(ips);
		}

		Mat4 offset;
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			offset = ips * val * ws;
			break;
		case TransformationSpace::TS_PARENT:
			offset = ips * val * ips * ws;
			break;
		case TransformationSpace::TS_LOCAL:
			offset = ips * val * ws;
			break;
		}

		DecomposeMatrix(offset, m_translation, m_orientation, m_scale);
	}

	void Node::SetTransform(const Mat4& val, TransformationSpace space)
	{
		assert(false && "Not implemented.");
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
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Mat4 ps, ws;
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ws[3].xyz = val;
			Mat4 ts = glm::inverse(ps) * ws;
			m_translation = ts[3].xyz;
		}
		break;
		case TransformationSpace::TS_PARENT:
			m_translation = val;
			break;
		case TransformationSpace::TS_LOCAL:
			Translate(val, TransformationSpace::TS_LOCAL);
		break;
		}
	}

	Vec3 Node::GetTranslation(TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ts = GetTransform(TransformationSpace::TS_WORLD);
				Vec3 t, s;
				Quaternion q;
				DecomposeMatrix(ts, t, q, s);
				return t;
			}
			return m_translation;
		case TransformationSpace::TS_PARENT:
			return m_translation;
		case TransformationSpace::TS_LOCAL:
		default:
			return Vec3();
		}
	}

	void Node::SetOrientation(const Quaternion& val, TransformationSpace space)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
		{
			Mat3 ps, ws;
			ws = GetTransform(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			Vec3 t, s;
			Quaternion q;
			DecomposeMatrix(ws, t, q, s);
			ws = glm::toMat3(val);
			ws = ws * glm::diagonal3x3(s);

			Mat3 ts = glm::inverse(ps) * ts;
			m_orientation = glm::toQuat(ts);
			m_orientation = glm::normalize(m_orientation);
		}
		break;
		case TransformationSpace::TS_PARENT:
			m_orientation = val;
			break;
		case TransformationSpace::TS_LOCAL:
			Rotate(val, TransformationSpace::TS_LOCAL);
			break;
		}
	}

	Quaternion Node::GetOrientation(TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ts = GetTransform(TransformationSpace::TS_WORLD);
				Vec3 t, s;
				Quaternion q;
				DecomposeMatrix(ts, t, q, s);
				return q;
			}
			return m_orientation;
		case TransformationSpace::TS_PARENT:
			return m_orientation;
		case TransformationSpace::TS_LOCAL:
		default:
			return Quaternion();
		}
	}

	void Node::SetScale(const Vec3& val, TransformationSpace space)
	{
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
		{
			Mat3 ts, ps;
			ts = glm::diagonal3x3(val);
			Quaternion ws = GetOrientation(TransformationSpace::TS_WORLD);
			if (m_parent != nullptr)
			{
				ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);
			}
			ts = glm::inverse(ps) * glm::toMat3(ws) * ts;
			Vec3 t;
			Quaternion q;
			DecomposeMatrix(ts, t, q, m_scale);
		}
		break;
		}
	}

	Vec3 Node::GetScale(TransformationSpace space) const
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ts = GetTransform(TransformationSpace::TS_WORLD);
				Vec3 t, s;
				Quaternion q;
				DecomposeMatrix(ts, t, q, s);
				return s;
			}
			return m_scale;
		case TransformationSpace::TS_PARENT:
			return m_scale;
		case TransformationSpace::TS_LOCAL:
		default:
			return Vec3(1.0f);
		}
	}

	void Node::AddChild(Node* child)
	{
		m_children.push_back(child);
		child->m_parent = this;
	}

}
