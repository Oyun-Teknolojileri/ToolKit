#include "stdafx.h"
#include "Node.h"
#include "MathUtil.h"
#include "DebugNew.h"

namespace ToolKit
{

	NodeId Node::m_nextId = 0;

	Node::Node()
	{
		m_id = ++m_nextId;
	}

	Node::~Node()
	{
		OrphanSelf();
		for (Node* child : m_children)
		{
			Orphan(child);
		}
	}

	void Node::Translate(const Vec3& val, TransformationSpace space)
	{
		Vec3 tmpScl = m_scale;
		m_scale = Vec3(1.0f);

		Mat4 ts = glm::translate(Mat4(), val);
		TransformImp(ts, space, &m_translation, nullptr, nullptr);

		m_scale = tmpScl;
	}

	void Node::Rotate(const Quaternion& val, TransformationSpace space)
	{
		Vec3 tmpScl = m_scale;
		m_scale = Vec3(1.0f);

		Mat4 ts = glm::toMat4(val);
		TransformImp(ts, space, nullptr, &m_orientation, nullptr);

		m_scale = tmpScl;
	}

	void Node::Scale(const Vec3& val, TransformationSpace space)
	{
		Mat4 ts = glm::diagonal4x4(Vec4(val, 1.0f));
		TransformImp(ts, space, nullptr, nullptr, &m_scale);
	}

	void Node::Transform(const Mat4& val, TransformationSpace space)
	{
		TransformImp(val, space, &m_translation, &m_orientation, &m_scale);
	}

	void Node::SetTransform(const Mat4& val, TransformationSpace space)
	{
		SetTransformImp(val, space, &m_translation, &m_orientation, &m_scale);
	}

	Mat4 Node::GetTransform(TransformationSpace space)
	{
		Mat4 ts;
		GetTransformImp(space, &ts, nullptr, nullptr, nullptr);
		return ts;
	}

	void Node::SetTranslation(const Vec3& val, TransformationSpace space)
	{
		if (m_parent == nullptr)
		{
			if (space == TransformationSpace::TS_LOCAL)
			{
				Translate(val, space);
			}
			else
			{
				m_translation = val;
			}
			SetChildrenDirty();
		}
		else
		{
			Mat4 ts = glm::translate(Mat4(), val);
			SetTransformImp(ts, space, &m_translation, nullptr, nullptr);
		}
	}

	Vec3 Node::GetTranslation(TransformationSpace space)
	{
		Vec3 t;
		GetTransformImp(space, nullptr, &t, nullptr, nullptr);
		return t;
	}

	void Node::SetOrientation(const Quaternion& val, TransformationSpace space)
	{
		if (m_parent == nullptr)
		{
			if (space == TransformationSpace::TS_LOCAL)
			{
				Rotate(val, space);
			}
			else
			{
				m_orientation = val;
			}
			SetChildrenDirty();
		}
		else
		{
			Mat4 ts = glm::toMat4(val);
			SetTransformImp(ts, space, nullptr, &m_orientation, nullptr);
		}
	}

	Quaternion Node::GetOrientation(TransformationSpace space)
	{
		Quaternion q;
		GetTransformImp(space, nullptr, nullptr, &q, nullptr);
		return q;
	}

	void Node::SetScale(const Vec3& val, TransformationSpace space)
	{
		SetChildrenDirty();

		// Unless locally applied, Scale needs to preserve directions. Apply rotation first.
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
			DecomposeMatrix(ts, nullptr, nullptr, &m_scale);
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
			DecomposeMatrix(ts, nullptr, nullptr, &m_scale);
		}
		break;
		case TransformationSpace::TS_LOCAL:
			m_scale = val;
			break;
		}
	}

	Vec3 Node::GetScale(TransformationSpace space)
	{
		Vec3 s;
		GetTransformImp(space, nullptr, nullptr, nullptr, &s);
		return s;
	}

	void Node::AddChild(Node* child)
	{
		assert(child->m_id != m_id);
		assert(child->m_parent == nullptr);
		// Preserve initial transform.
		Mat4 ts = child->GetTransform(TransformationSpace::TS_WORLD);

		m_children.push_back(child);
		child->m_parent = this;
		child->m_dirty = true;
		child->SetChildrenDirty();

		child->SetTransform(ts, TransformationSpace::TS_WORLD);
	}

	void Node::Orphan(Node* child)
	{
		for (size_t i = 0; i < m_children.size(); i++)
		{
			if (m_children[i] == child)
			{
				// Preserve initial transform.
				Mat4 ts = child->GetTransform(TransformationSpace::TS_WORLD);

				child->m_parent = nullptr;
				child->m_dirty = true;
				child->SetChildrenDirty();
				m_children.erase(m_children.begin() + i);

				child->SetTransform(ts, TransformationSpace::TS_WORLD);
				return;
			}
		}
	}

	void Node::OrphanSelf()
	{
		if (m_parent)
		{
			m_parent->Orphan(this);
		}
	}

	Node* Node::GetRoot() const
	{
		if (m_parent == nullptr)
		{
			return nullptr;
		}

		return m_parent->GetRoot();
	}

	Node* Node::GetCopy() const
	{
		Node* node = new Node();
		if (m_parent != nullptr)
		{
			m_parent->AddChild(node);
		}

		node->m_inheritScale = m_inheritScale;
		node->m_inheritOnlyTranslate = m_inheritOnlyTranslate;
		// m_children // No Copy.
		node->m_translation = m_translation;
		node->m_orientation = m_orientation;
		node->m_scale = m_scale;

		return node;
	}

	void Node::Serialize(XmlDocument* doc, XmlNode* parent) const
	{
		XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlNodeElement.c_str());
		if (parent != nullptr)
		{
			parent->append_node(node);
		}
		else
		{
			doc->append_node(node);
		}
		
		WriteAttr(node, doc, XmlNodeIdAttr, std::to_string(m_id));

		if (m_parent != nullptr)
		{
			WriteAttr(node, doc, XmlNodeParentIdAttr, std::to_string(m_parent->m_id));
		}

		WriteAttr(node, doc, XmlNodeInheritScaleAttr, std::to_string((int)m_inheritScale));
		WriteAttr(node, doc, XmlNodeInheritTranslateOnlyAttr, std::to_string((int)m_inheritOnlyTranslate));

		XmlNode* tNode = doc->allocate_node(rapidxml::node_element, XmlTranslateElement.c_str());
		WriteXYZ(tNode, doc, m_translation);
		node->append_node(tNode);

		tNode = doc->allocate_node(rapidxml::node_element, XmlRotateElement.c_str());
		WriteXYZW(tNode, doc, m_orientation);
		node->append_node(tNode);

		tNode = doc->allocate_node(rapidxml::node_element, XmlScaleElement.c_str());
		WriteXYZ(tNode, doc, m_scale);
		node->append_node(tNode);
	}

	void Node::DeSerialize(XmlDocument* doc, XmlNode* parent)
	{
		XmlNode* node = parent;
		if (node == nullptr)
		{
			assert(false && "Unbound node can not exist.");
			return;
		}

		if (XmlAttribute* id = node->first_attribute(XmlNodeIdAttr.c_str()))
		{
			String val = id->value();
			m_id = std::atoi(val.c_str());
		}

		if (XmlAttribute* pid = node->first_attribute(XmlNodeParentIdAttr.c_str()))
		{
			String val = pid->value();
			// Node look up from parent ...
		}

		if (XmlAttribute* attr = node->first_attribute(XmlNodeInheritScaleAttr.c_str()))
		{
			String val = attr->value();
			m_inheritScale = (bool)std::atoi(val.c_str());
		}

		if (XmlAttribute* attr = node->first_attribute(XmlNodeInheritTranslateOnlyAttr.c_str()))
		{
			String val = attr->value();
			m_inheritOnlyTranslate = (bool)std::atoi(val.c_str());
		}

		if (XmlNode* n = node->first_node(XmlTranslateElement.c_str()))
		{
			ExtractXYZFromNode(n, m_translation);
		}

		if (XmlNode* n = node->first_node(XmlRotateElement.c_str()))
		{
			ExtractQuatFromNode(n, m_orientation);
		}

		if (XmlNode* n = node->first_node(XmlScaleElement.c_str()))
		{
			ExtractXYZFromNode(n, m_scale);
		}
	}

	void Node::TransformImp(const Mat4& val, TransformationSpace space, Vec3* translation, Quaternion* orientation, Vec3* scale)
	{
		Mat4 ps, ts;
		ps = GetParentTransform();
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			ts = glm::inverse(ps) * val * ps * GetLocalTransform();
			break;
		case TransformationSpace::TS_PARENT:
			ts = val * GetLocalTransform();
			break;
		case TransformationSpace::TS_LOCAL:
			ts = GetLocalTransform() * val;
			break;
		}

		DecomposeMatrix(ts, translation, orientation, scale);
		SetChildrenDirty();
	}

	void Node::SetTransformImp(const Mat4& val, TransformationSpace space, Vec3* translation, Quaternion* orientation, Vec3* scale)
	{
		Mat4 ts;
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ps = GetParentTransform();
				ts = glm::inverse(ps) * val;
				break;
			} // Fall trough.
		case TransformationSpace::TS_PARENT:
			ts = val;
			break;
		case TransformationSpace::TS_LOCAL:
			Transform(val, TransformationSpace::TS_LOCAL);
			return;
		}

		DecomposeMatrix(ts, translation, orientation, scale);
		SetChildrenDirty();
	}

	void Node::GetTransformImp(TransformationSpace space, Mat4* transform, Vec3* translation, Quaternion* orientation, Vec3* scale)
	{
		switch (space)
		{
		case TransformationSpace::TS_WORLD:
			if (m_parent != nullptr)
			{
				Mat4 ts = GetLocalTransform();
				Mat4 ps = GetParentTransform();
				ts = ps * ts;
				if (transform != nullptr)
				{
					*transform = ts;
				}
				DecomposeMatrix(ts, translation, orientation, scale);
				break;
			} // Fall trough.
		case TransformationSpace::TS_PARENT:
			if (transform != nullptr)
			{
				*transform = GetLocalTransform();
			}
			if (translation != nullptr)
			{
				*translation = m_translation;
			}
			if (orientation != nullptr)
			{
				*orientation = m_orientation;
			}
			if (scale != nullptr)
			{
				*scale = m_scale;
			}
			break;
		case TransformationSpace::TS_LOCAL:
		default:
			if (transform != nullptr)
			{
				*transform = Mat4();
			}
			if (translation != nullptr)
			{
				*translation = Vec3();
			}
			if (orientation != nullptr)
			{
				*orientation = Quaternion();
			}
			if (scale != nullptr)
			{
				*scale = Vec3(1.0f);
			}
			break;
		}
	}

	ToolKit::Mat4 Node::GetLocalTransform() const
	{
		Mat4 ts = glm::toMat4(m_orientation);
		ts = glm::scale(ts, m_scale);
		ts[3].xyz = m_translation;
		return ts;
	}

	Mat4 Node::GetParentTransform()
	{
		Mat4 ps;
		if (m_parent != nullptr)
		{
			if (!m_dirty)
			{
				return m_parentCache;
			}

			ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);

			if (m_inheritOnlyTranslate)
			{
				Vec3 t = ps[3];
				ps = glm::translate(Mat4(), t);
			}
			else if (!m_inheritScale)
			{
				for (int i = 0; i < 3; i++)
				{
					Vec3 v = ps[i];
					ps[i].xyz = glm::normalize(v);
				}
			}
			
			m_parentCache = ps;
		}

		m_dirty = false;
		return ps;
	}

	void Node::SetChildrenDirty()
	{
		for (Node* c : m_children)
		{
			c->m_dirty = true;
			c->SetChildrenDirty();
		}
	}

}
