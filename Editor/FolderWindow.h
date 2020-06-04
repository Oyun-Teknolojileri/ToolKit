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

		class FolderWindow;

		class FolderView
		{
		public:
			FolderView();
			FolderView(FolderWindow* parent);

			void Show();
			void SetPath(const String& path);
			const String& GetPath() const;
			void Iterate();

		private:
			FolderWindow* m_parent = nullptr;
			std::vector<DirectoryEntry> m_entiries;
			String m_path;
			String m_folder;

		public:
			bool m_visible = true;
			bool m_onlyNativeTypes = true;
		};

		class FolderWindow : public Window
		{
		public:
			FolderWindow();
			virtual void Show() override;
			virtual Type GetType() override;
			void Iterate(const String& path);
			void AddEntry(const FolderView& view);
			FolderView& GetView(int indx);
			int Exist(const String& path);

		private:
			std::vector<FolderView> m_entiries;
			String m_path;
		};

	}
}