#pragma once

#include "ToolKit.h"
#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{
		class Viewport;

		class OverlayNav
		{
		public:
			OverlayNav(Viewport* owner);
			void Show();

		public:
			Viewport* m_owner;
		};
	}
}
