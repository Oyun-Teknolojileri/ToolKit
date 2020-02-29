#pragma once

#include "ToolKit.h"

namespace ToolKit
{
	namespace Editor
	{
		class BaseMod
		{
		public:
			virtual void Init();
			virtual void StateBegin();
			virtual void StateProcess();
			virtual void StateEnd();

		public:
			int m_state; // While state is 0. It remains
		};

		class SelectMod : public BaseMod
		{
		public:
			virtual void Init() override;

		};
	}
}
