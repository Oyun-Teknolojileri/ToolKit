#pragma once

#include <string>
#include <unordered_map>
#include "Drawable.h"
#include "StateMachine.h"

namespace ToolKit
{
  class Animation;
  class StateMachine;
  class Actor;

	class ActorState : public State
	{
	public:
		ActorState(Actor* actor);
		virtual ~ActorState();

	public:
		Animation* m_currentAnim = nullptr;
		Actor* m_actor;
	};

  class Actor : public Drawable
  {
  public:
    Actor();
    virtual ~Actor();
    virtual EntityType GetType() const override;

  public:
    StateMachine* m_stateMachine;
  };
}
