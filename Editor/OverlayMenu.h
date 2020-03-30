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
			void ShowOverlayNav();

		public:
			Viewport* m_owner;
		};
	}
}
