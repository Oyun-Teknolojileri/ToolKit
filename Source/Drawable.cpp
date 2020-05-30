#include "stdafx.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Material.h"
#include "ToolKit.h"
#include "Node.h"
#include "DebugNew.h"

namespace ToolKit
{

	Drawable::Drawable()
	{
		m_mesh = std::make_shared<Mesh>();
	}

	Drawable::~Drawable()
	{
	}

	bool Drawable::IsDrawable() const
	{
		return true;
	}

	EntityType Drawable::GetType() const
	{
		return EntityType::Entity_Drawable;
	}

	void Drawable::SetPose(Animation* anim)
	{
		if (m_mesh->IsSkinned())
		{
			Skeleton* skeleton = ((SkinMesh*)m_mesh.get())->m_skeleton;
			anim->GetCurrentPose(skeleton);
		}
		else
		{
			anim->GetCurrentPose(m_node);
		}
	}

	BoundingBox Drawable::GetAABB(bool inWorld) const
	{
		BoundingBox bb = m_mesh->m_aabb;
		if (inWorld)
		{
			TransformAABB(bb, m_node->GetTransform(TransformationSpace::TS_WORLD));
		}

		return bb;
	}

	Drawable* Drawable::GetCopy() const
	{
		Drawable* cpy = new Drawable();
		GetCopy(cpy);

		return cpy;
	}

	void Drawable::GetCopy(Entity* copyTo) const
	{
		Entity::GetCopy(copyTo);
		Drawable* ntt = static_cast<Drawable*> (copyTo);
		ntt->m_mesh = MeshPtr(m_mesh->GetCopy());
	}

	static const String XmlMeshStr("M");
	static const String XmlFileAttr("f");

	void Drawable::Serialize(XmlDocument* doc, XmlNode* parent) const
	{
		Entity::Serialize(doc, parent);
		XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlMeshStr.c_str());
		node->append_attribute(doc->allocate_attribute(XmlFileAttr.c_str(), m_mesh->m_file.c_str()));
		parent->last_node()->append_node(node);
	}

}
