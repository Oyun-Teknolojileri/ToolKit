#pragma once

#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{

		class FolderView
		{
		public:
			void Show();
			void SetPath(const String& path);
			void Iterate();

		private:
			StringArray m_entiries;
			String m_path;
			String m_folder;
		};

		class FolderWindow : public Window
		{
		public:
			FolderWindow();
			virtual void Show() override;
			virtual Type GetType() override;
			void Iterate(const String& path);

		private:
			std::vector<FolderView> m_entiries;
			String m_path;
		};

	}
}