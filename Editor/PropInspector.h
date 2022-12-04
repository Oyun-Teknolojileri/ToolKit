#pragma once

#include "FolderWindow.h"
#include "UI.h"

#include <functional>
#include <vector>

namespace ToolKit
{
  namespace Editor
  {
    void DropZone(uint fallbackIcon,
                  const String& file,
                  std::function<void(const DirectoryEntry& entry)> dropAction,
                  const String& dropName = "");

    void DropSubZone(
        const String& title,
        uint fallbackIcon,
        const String& file,
        std::function<void(const DirectoryEntry& entry)> dropAction,
        bool isEditable);

    class View
    {
     public:
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

    class PrefabView : public View
    {
     public:
      PrefabView();
      virtual ~PrefabView();
      virtual void Show();

     private:
      bool DrawHeader(Entity* ntt, ImGuiTreeNodeFlags flags);
      void ShowNode(Entity* e);

     private:
      Entity* m_activeChildEntity = nullptr;
    };

    class EntityView : public View
    {
     public:
      EntityView();
      virtual ~EntityView();
      virtual void Show();
      virtual void ShowParameterBlock();

     protected:
      void ShowAnchorSettings();
    };

    class ComponentView : public View
    {
     public:
      ComponentView();
      virtual ~ComponentView();
      virtual void Show();
    };

    class CustomDataView : public View
    {
     public:
      CustomDataView();
      virtual ~CustomDataView();
      virtual void Show();
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
