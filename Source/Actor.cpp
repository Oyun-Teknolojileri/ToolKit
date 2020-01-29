#include "stdafx.h"
#include "Actor.h"
#include "ToolKit.h"
#include "DebugNew.h"

ToolKit::SignaleId::SignaleId(int id)
{
  m_id = id;
}

ToolKit::State::State(Actor* actor)
  : m_currentSignale(-1)
{
  m_actor = actor;
}

ToolKit::State::~State()
{
  SafeDel(m_currentAnim);
}

ToolKit::StateMachine::~StateMachine()
{
  for (auto& state : m_states)
    SafeDel(state.second);
}

void ToolKit::StateMachine::Signale(SignaleId signale)
{
  State* nextState = m_currentState->Signaled(signale);
  
  if (nextState == nullptr)
    return;

  m_currentState->TransitionOut(nextState);
  nextState->TransitionIn(m_currentState);
  m_currentState = nextState;
}

bool ToolKit::StateMachine::QueryState(std::string stateName)
{
  return m_states.find(stateName) != m_states.end();
}

ToolKit::Actor::Actor()
{
  m_stateMachine = new StateMachine();
}

ToolKit::Actor::~Actor()
{
  SafeDel(m_stateMachine);
}

ToolKit::EntityType ToolKit::Actor::GetType()
{
  return EntityType::Entity_Actor;
}
