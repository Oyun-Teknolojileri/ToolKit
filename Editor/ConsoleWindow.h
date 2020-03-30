#pragma once

#include "ToolKit.h"
#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{
		class ConsoleWindow
		{
		public:
			void Show();

		private:
			// States.
			bool m_open = true;
		};
	}
}