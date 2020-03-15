#include "stdafx.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Material.h"
#include "ToolKit.h"
#include "Node.h"
#include "DebugNew.h"

ToolKit::Drawable::Drawable()
{
  m_mesh = std::shared_ptr<Mesh>(new Mesh());
}

ToolKit::Drawable::~Drawable()
{
}

bool ToolKit::Drawable::IsDrawable()
{
	return true;
}

ToolKit::EntityType ToolKit::Drawable::GetType()
{
  return EntityType::Entity_Drawable;
}

void ToolKit::Drawable::SetPose(Animation* anim)
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

ToolKit::BoundingBox ToolKit::Drawable::GetAABB(bool inWorld)
{
	BoundingBox bb = m_mesh->m_aabb;

	if (inWorld)
	{
		TransformAABB(bb, m_node->GetTransform());
	}

	return m_mesh->m_aabb;
}
