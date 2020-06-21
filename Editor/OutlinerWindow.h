#pragma once

#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{
		class OutlinerWindow : public Window
		{
		public:
			OutlinerWindow();
			virtual void Show() override;
			virtual Type GetType() const override;
		};
	}
}
