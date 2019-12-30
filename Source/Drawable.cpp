#include "stdafx.h"
#include "Drawable.h"
#include "Mesh.h"
#include "Material.h"
#include "ToolKit.h"

ToolKit::Drawable::Drawable()
{
  m_mesh = std::shared_ptr<Mesh> (new Mesh());
}

ToolKit::Drawable::~Drawable()
{
}

ToolKit::EntityType ToolKit::Drawable::GetType()
{
  return EntityType::Entity_Drawable;
}

bool ToolKit::Drawable::IsDrawable()
{
  return true;
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
