/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "StateMachine.h"

#include "ToolKit.h"

#include "DebugNew.h"

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
