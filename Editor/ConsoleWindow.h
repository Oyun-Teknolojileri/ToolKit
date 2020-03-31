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
			void SetVisibility(bool visible);

			// Window queries.
			bool IsOpen();

		private:
			// States.
			bool m_open = true;
		};
	}
}