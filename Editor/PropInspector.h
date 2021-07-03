#pragma once

#include "UI.h"
#include "FolderWindow.h"
#include <functional>

namespace ToolKit
{
  namespace Editor
  {

    class View 
    {
    public:
      virtual ~View() {}
      virtual void Show() = 0;

      void DropZone(uint fallbackIcon, const String& file, std::function<void(const DirectoryEntry& entry)> dropAction);
      void DropSubZone(uint fallbackIcon, const String& file, std::function<void(const DirectoryEntry& entry)> dropAction);

    public:
      Entity* m_entity = nullptr;
      int m_viewID = 0;
    };

    class EntityView : public View
    {
    public:
      EntityView() { m_viewID = 1;  }
      virtual ~EntityView() {}
      virtual void Show();
    };

    class MeshView : public View
    {
    public:
      MeshView() { m_viewID = 2; }
      virtual ~MeshView() {}
      virtual void Show() override;
    };

    class MaterialView : public View
    {
    public:
      MaterialView() { m_viewID = 3; }
      virtual ~MaterialView() {}
      virtual void Show() override;
    };

    class PropInspector : public Window
    {
    public:
      PropInspector();
      virtual ~PropInspector();

      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void DispatchSignals() const override;

      template<typename T>
      T* GetView()
      {
        for (View* v : m_views)
        {
          if (T* cv = dynamic_cast<T*> (v))
          {
            return cv;
          }
        }

        assert(false && "Invalid View type queried");
        return nullptr;
      }

    public:
      std::vector<View*> m_views;
    };

  }
}