#pragma once

#include <unordered_map>

namespace ToolKit
{
	class SignalId
	{
	public:
		SignalId(int id);
		bool operator== (const SignalId& rhs);

	public:
		int m_id;
	};

	class State
	{
	public:
		State(std::string name);
		virtual ~State();
		virtual void TransitionIn(State* prevState) = 0;
		virtual void TransitionOut(State* nextState) = 0;
		virtual void Update(float deltaTime) = 0;
		virtual std::string Signaled(SignalId signal) = 0; // Returns next state's name.

	public:
		const std::string m_name;
		std::unordered_map<int, std::string> m_links; // Specific signals might jump to a state. This provides the hijacking mechanism.
	};

	class StateMachine
	{
	public:
		StateMachine();
		~StateMachine();

		void Signal(SignalId signal);
		State* QueryState(std::string stateName);
		void PushState(State* state);
		void Update(float deltaTime);

	public:
		State* m_currentState;

	private:
		std::unordered_map<std::string, State*> m_states;
	};
}
