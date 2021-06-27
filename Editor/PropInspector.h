#pragma once

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
    };

    class AssetView : public View
    {
    public:
      virtual ~AssetView() {}
      virtual void Show() override;

    public:
      DirectoryEntry m_entry;
    };

    class EntityView : public View
    {
    public:
      virtual ~EntityView() {}
      virtual void Show();

    public:
      Entity* m_entry;
    };

    class MeshView : public View
    {
    public:
      virtual ~MeshView() {}
      virtual void Show() override;

    public:
      MeshPtr m_entry;
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