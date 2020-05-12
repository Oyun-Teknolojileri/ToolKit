#pragma once

#include "ToolKit.h"
#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{
		class Viewport;

		class OverlayUI
		{
		public:
			OverlayUI(Viewport* owner);
			virtual ~OverlayUI();
			virtual void Show() = 0;

		public:
			Viewport* m_owner;
		};

		class OverlayMods : public OverlayUI
		{
		public:
			OverlayMods(Viewport* owner);
			virtual void Show() override;
		};

		class OverlayViewportOptions : public OverlayUI
		{
		public:
			OverlayViewportOptions(Viewport* owner);
			virtual void Show() override;
		};

	}
}
