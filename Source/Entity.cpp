#include "stdafx.h"
#include "Entity.h"
#include "Entity.h"
#include "Node.h"
#include "ToolKit.h"
#include "Skeleton.h"
#include "DebugNew.h"

ToolKit::EntityId ToolKit::Entity::m_lastId = 0;

ToolKit::Entity::Entity()
{
  m_node = new Node();
	m_id = m_lastId++;
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
