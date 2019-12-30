#include "stdafx.h"
#include "Entity.h"
#include "Entity.h"
#include "Node.h"
#include "ToolKit.h"
#include "Skeleton.h"

ToolKit::Entity::Entity()
{
  m_node = new Node();
}

ToolKit::Entity::~Entity()
{
  SafeDel(m_node);
}

bool ToolKit::Entity::IsDrawable()
{
  return false;
}

ToolKit::EntityType ToolKit::Entity::GetType()
{
  return EntityType::Entity_Base;
}

void ToolKit::Entity::SetPose(Animation* anim)
{
  anim->GetCurrentPose(m_node);
}
