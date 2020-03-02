#include "stdafx.h"
#include "Mod.h"

ToolKit::Editor::BaseMod::BaseMod()
{
	m_stateMachine = new StateMachine();
}

ToolKit::Editor::BaseMod::~BaseMod()
{
	SafeDel(m_stateMachine);
}

void ToolKit::Editor::BaseMod::Init()
{
}

void ToolKit::Editor::BaseMod::Update(float deltaTime)
{
}

void ToolKit::Editor::StateBeginPick::Update(float deltaTime)
{
}

ToolKit::State* ToolKit::Editor::StateBeginPick::Signaled(SignalId signal)
{
	return nullptr;
}

void ToolKit::Editor::StateBeginBoxPick::Update(float deltaTime)
{
}

ToolKit::State* ToolKit::Editor::StateBeginBoxPick::Signaled(SignalId signal)
{
	return nullptr;
}

void ToolKit::Editor::StateEndPick::Update(float deltaTime)
{
}

ToolKit::State* ToolKit::Editor::StateEndPick::Signaled(SignalId signal)
{
	return nullptr;
}

void ToolKit::Editor::SelectMod::Init()
{
}

void ToolKit::Editor::SelectMod::Update(float deltaTime)
{
}
