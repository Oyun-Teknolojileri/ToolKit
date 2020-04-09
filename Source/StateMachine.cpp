#include "stdafx.h"
#include "ToolKit.h"
#include "StateMachine.h"
#include "DebugNew.h"

ToolKit::State::State(std::string name)
	: m_name(name)
{
}

ToolKit::State::~State()
{
}

ToolKit::StateMachine::StateMachine()
{
	m_currentState = nullptr;
}

ToolKit::StateMachine::~StateMachine()
{
	for (auto& state : m_states)
	{
		SafeDel(state.second);
	}
}

void ToolKit::StateMachine::Signal(SignalId signal)
{
	if (m_currentState == nullptr)
	{
		return;
	}

	std::string query = m_currentState->Signaled(signal);
	if (query.empty())
	{
		// If the signal is not processed in the current state, check links for hijack.
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

ToolKit::State* ToolKit::StateMachine::QueryState(std::string stateName)
{
	if (m_states.find(stateName) != m_states.end())
	{
		return m_states[stateName];
	}

	return nullptr;
}

void ToolKit::StateMachine::PushState(State* state)
{
	assert(m_states.find(state->m_name) == m_states.end()); // Make sure states are unique.
	m_states[state->m_name] = state;
}

void ToolKit::StateMachine::Update(float deltaTime)
{
	if (m_currentState != nullptr)
	{
		m_currentState->Update(deltaTime);
	}
}
