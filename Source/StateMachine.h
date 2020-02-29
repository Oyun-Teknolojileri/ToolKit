#pragma once

#include <unordered_map>

namespace ToolKit
{
	class SignalId
	{
	public:
		SignalId(int id);
		int m_id;
	};

	class State
	{
	public:
		State();
		~State();
		virtual void TransitionIn(State* prevState) = 0;
		virtual void TransitionOut(State* nextState) = 0;
		virtual void Update(float deltaTime) = 0;
		virtual State* Signaled(SignalId signale) = 0;

	public:
		SignalId m_currentSignal;
	};

	class StateMachine
	{
	public:
		~StateMachine();
		void Signal(SignalId signal);
		bool QueryState(std::string stateName);

	public:
		std::unordered_map<std::string, State*> m_states;
		State* m_currentState;
	};
}
