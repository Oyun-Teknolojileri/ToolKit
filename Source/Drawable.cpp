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
		m_mesh = std::shared_ptr<Mesh>(new Mesh());
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
			TransformAABB(bb, m_node->GetTransform());
		}

		return bb;
	}

}
