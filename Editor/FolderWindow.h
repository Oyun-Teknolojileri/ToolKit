#pragma once

#include "UI.h"

namespace ToolKit
{
	namespace Editor
	{

		struct DirectoryEntry
		{
			String m_ext;
			String m_fileName;
			String m_rootPath;
			bool m_isDirectory = false;
		};

		class FolderView
		{
		public:
			void Show();
			void SetPath(const String& path);
			void Iterate();

		private:
			std::vector<DirectoryEntry> m_entiries;
			String m_path;
			String m_folder;

		public:
			bool m_visible = true;
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