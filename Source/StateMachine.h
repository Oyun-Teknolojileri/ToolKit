#pragma once

#include "ToolKit.h"

namespace ToolKit
{

  class State
  {
  public:
    State();
    virtual ~State();
    virtual void TransitionIn(State* prevState) = 0;
    virtual void TransitionOut(State* nextState) = 0;
    virtual SignalId Update(float deltaTime) = 0; // Returns self signal. Update may need to initiate self signals. State machine checks self signals and call them on the state.
    virtual String Signaled(SignalId signal) = 0; // Returns next state's name.
    virtual String GetType() = 0;

    template<typename T>
    bool ThisIsA() { return typeid(*this) == typeid(T); }

  public:
    std::unordered_map<int, String> m_links; // Specific signals might jump to a state. This provides the hijacking mechanism.
    static SignalId NullSignal = -1; // StateMachine does not take any action.
  };

  class StateMachine
  {
  public:
    StateMachine();
    ~StateMachine();

    void Signal(SignalId signal);
    State* QueryState(String type);
    void PushState(State* state);
    void Update(float deltaTime);

  public:
    State* m_currentState;

  private:
    std::unordered_map<String, State*> m_states;
  };
}
