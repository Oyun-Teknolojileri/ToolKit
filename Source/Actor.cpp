#include "stdafx.h"
#include "Actor.h"
#include "ToolKit.h"
#include "DebugNew.h"

ToolKit::ActorState::ActorState(Actor* actor)
  : State("Actor")
{
  m_actor = actor;
}

ToolKit::ActorState::~ActorState()
{
  SafeDel(m_currentAnim);
}

ToolKit::Actor::Actor()
{
  m_stateMachine = new StateMachine();
}

ToolKit::Actor::~Actor()
{
  SafeDel(m_stateMachine);
}

ToolKit::EntityType ToolKit::Actor::GetType() const
{
  return EntityType::Entity_Actor;
}
