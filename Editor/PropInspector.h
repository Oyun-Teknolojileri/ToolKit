#pragma once

#include <vector>
#include <functional>

#include "UI.h"
#include "FolderWindow.h"

namespace ToolKit
{
  namespace Editor
  {

    class View
    {
     public:
      virtual ~View() {}
      virtual void Show() = 0;
      virtual void ShowVariant(ParameterVariant* var);

      void DropZone
      (
        uint fallbackIcon,
        const String& file,
        std::function<void(const DirectoryEntry& entry)> dropAction,
        const String& dropName = ""
      );

      void DropSubZone
      (
        const String& title,
        uint fallbackIcon,
        const String& file,
        std::function<void(const DirectoryEntry& entry)> dropAction
      );

     protected:
      bool ImGuiEnterPressed();

     public:
      Entity* m_entity = nullptr;
      int m_viewID = 0;
    };

    class EntityView : public View
    {
     public:
      EntityView() { m_viewID = 1; }
      virtual ~EntityView() {}
      virtual void Show();
      virtual void ShowParameterBlock(ParameterBlock& params, ULongID id);
      virtual bool ShowComponentBlock(ParameterBlock& params, ULongID id);

     protected:
      void ShowCustomData();
    };

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
      EntityView* m_view;
    };

  }  // namespace Editor
}  // namespace ToolKit
