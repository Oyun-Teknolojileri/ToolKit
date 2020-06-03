#pragma once

#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{

		class FolderWindow : public Window
		{
		public:
			FolderWindow();
			virtual void Show() override;
			virtual Type GetType() override;
			void Iterate(const String& path);

		private:
			StringArray m_entiries;
		};

	}
}