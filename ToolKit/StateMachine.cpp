/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "StateMachine.h"

#include "ToolKit.h"



namespace ToolKit
{

  State::State() {}

  State::~State() {}

  StateMachine::StateMachine() { m_currentState = nullptr; }

  StateMachine::~StateMachine()
  {
    for (auto& state : m_states)
    {
      SafeDel(state.second);
    }
  }

  void StateMachine::Signal(SignalId signal)
  {
    if (m_currentState == nullptr)
    {
      return;
    }

    String query = m_currentState->Signaled(signal);
    if (query.empty())
    {
      // If the signal is not processed in the current state
      // check links for hijack.
      auto link = m_currentState->m_links.find(signal);
      if (link != m_currentState->m_links.end())
      {
        query = link->second;
      }
    }

    State* nextState = QueryState(query);
    if (nextState == nullptr)
    {
      return;
    }

    m_currentState->TransitionOut(nextState);
    nextState->TransitionIn(m_currentState);
    m_currentState = nextState;
  }

  State* StateMachine::QueryState(const String& type)
  {
    if (m_states.find(type) != m_states.end())
    {
      return m_states[type];
    }

    return nullptr;
  }

  void StateMachine::PushState(State* state)
  {
    // Make sure states are unique.
    assert(m_states.find(state->GetType()) == m_states.end());
    m_states[state->GetType()] = state;
  }

  void StateMachine::Update(float deltaTime)
  {
    if (m_currentState != nullptr)
    {
      SignalId selfSig = m_currentState->Update(deltaTime);
      if (selfSig != State::NullSignal)
      {
        Signal(selfSig);
      }
    }
  }

} // namespace ToolKit
