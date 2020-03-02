#include "stdafx.h"
#include "ToolKit.h"
#include "StateMachine.h"
#include "DebugNew.h"

ToolKit::SignalId::SignalId(int id)
{
	m_id = id;
}

bool ToolKit::SignalId::operator== (const SignalId& rhs)
{ 
	return m_id == rhs.m_id; 
}

ToolKit::State::State()
	: m_currentSignal(-1)
{
}

ToolKit::State::~State()
{
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
	State* nextState = m_currentState->Signaled(signal);

	if (nextState == nullptr)
	{
		return;
	}

	m_currentState->TransitionOut(nextState);
	nextState->TransitionIn(m_currentState);
	m_currentState = nextState;
}

bool ToolKit::StateMachine::QueryState(std::string stateName)
{
	return m_states.find(stateName) != m_states.end();
}
