#include "stdafx.h"
#include "Move.h"
#include "Gizmo.h"
#include "GlobalDef.h"
#include "Node.h"
#include "DebugNew.h"

ToolKit::Editor::StateMoveBase::StateMoveBase(std::string name)
	: State(name)
{
}

void ToolKit::Editor::StateMoveBase::TransitionIn(State* prevState)
{
}

void ToolKit::Editor::StateMoveBase::TransitionOut(State* nextState)
{
	StateMoveBase* baseState = dynamic_cast<StateMoveBase*> (nextState);
	if (baseState != nullptr)
	{
		baseState->m_gizmo = m_gizmo;
	}
}

void ToolKit::Editor::StateBeginMove::TransitionIn(State* prevState)
{
}

void ToolKit::Editor::StateBeginMove::TransitionOut(State* nextState)
{
}

void ToolKit::Editor::StateBeginMove::Update(float deltaTime)
{
	if (g_app->m_scene.GetSelectedEntityCount() > 0)
	{
		if (m_gizmo == nullptr)
		{
			m_gizmo = std::make_shared<MoveGizmo>();
			g_app->m_scene.AddEntity(m_gizmo.get());
		}

		Entity* e = g_app->m_scene.GetCurrentSelection();
		m_gizmo->m_worldLocation = e->m_node->GetTranslation(TransformationSpace::TS_WORLD);
		m_gizmo->m_node->m_orientation = e->m_node->GetOrientation(TransformationSpace::TS_WORLD);
		m_gizmo->Update(deltaTime);
	}
	else
	{
		if (m_gizmo != nullptr)
		{
			g_app->m_scene.RemoveEntity(m_gizmo->m_id);
		}
	}
}

std::string ToolKit::Editor::StateBeginMove::Signaled(SignalId signal)
{
	return "";
}

ToolKit::Editor::MoveMod::~MoveMod()
{
	if (m_stateMachine->m_currentState != nullptr)
	{
		StateMoveBase* baseState = dynamic_cast<StateMoveBase*> (m_stateMachine->m_currentState);
		if (baseState->m_gizmo != nullptr)
		{
			g_app->m_scene.RemoveEntity(baseState->m_gizmo->m_id);
		}
	}
}

void ToolKit::Editor::MoveMod::Init()
{
	StateBeginMove* initialState = new StateBeginMove();
	m_stateMachine->m_currentState = initialState;

	m_stateMachine->PushState(initialState);
}

void ToolKit::Editor::MoveMod::Update(float deltaTime)
{
	BaseMod::Update(deltaTime);
}
