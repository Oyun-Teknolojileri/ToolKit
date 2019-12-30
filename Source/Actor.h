#pragma once

#include <string>
#include <unordered_map>
#include "Drawable.h"

namespace ToolKit
{
  class Animation;
  class StateMachine;
  class Actor;

  class SignaleId
  {
  public:
    SignaleId(int id);
    int m_id;
  };

  class State
  {
  public:
    State(Actor* actor);
    virtual ~State();
    virtual void TransitionIn(State* prevState) = 0;
    virtual void TransitionOut(State* nextState) = 0;
    virtual void Update(float deltaTime) = 0;
    virtual State* Signaled(SignaleId signale) = 0;

  public:
    Animation* m_currentAnim = nullptr;
    SignaleId m_currentSignale;
    Actor* m_actor;
  };

  class StateMachine
  {
  public:
    ~StateMachine();
    void Signale(SignaleId signale);
    bool QueryState(std::string stateName);

  public:
    std::unordered_map<std::string, State*> m_states;
    State* m_currentState;
  };

  class Actor : public Drawable
  {
  public:
    Actor();
    virtual ~Actor();
    virtual EntityType GetType();

  public:
    StateMachine* m_stateMachine;
  };
}
