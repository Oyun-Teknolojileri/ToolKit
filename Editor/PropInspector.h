#pragma once

#include "FolderWindow.h"
#include "UI.h"

#include <functional>
#include <vector>

namespace ToolKit
{
  namespace Editor
  {

    class View
    {
     public:
      static void DropZone(uint fallbackIcon,
                    const String& file,
                    std::function<void(const DirectoryEntry& entry)> dropAction,
                    const String& dropName = "");
      static void DropSubZone(
          const String& title,
          uint fallbackIcon,
          const String& file,
          std::function<void(const DirectoryEntry& entry)> dropAction,
          bool isEditable);
      static bool IsTextInputFinalized();

      View(const StringView viewName);
      virtual ~View()
      {
      }
      virtual void Show() = 0;

     public:
      Entity* m_entity     = nullptr;
      int m_viewID         = 0;
      TexturePtr m_viewIcn = nullptr;
      const StringView m_viewName;
    };

    typedef View* ViewRawPtr;
    typedef std::vector<ViewRawPtr> ViewRawPtrArray;
    class PropInspector : public Window
    {
     public:
      explicit PropInspector(XmlNode* node);
      PropInspector();
      virtual ~PropInspector();

      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;

     public:
      ViewRawPtrArray m_views;
      uint m_activeViewIndx = 0;
    };
  } // namespace Editor
} // namespace ToolKit
