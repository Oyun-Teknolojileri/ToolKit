#pragma once

#include "EditorViewport.h"
#include "FolderWindow.h"
#include "Pass.h"
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
      static void DropZone(
          uint fallbackIcon,
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

      virtual ~View() {}

      virtual void Show() = 0;

     public:
      Entity* m_entity     = nullptr;
      int m_viewID         = 0;
      TexturePtr m_viewIcn = nullptr;
      const StringView m_viewName;
    };

    class PreviewViewport : public EditorViewport
    {
     public:
      PreviewViewport(uint width, uint height);
      ~PreviewViewport();
      void Show() override;
      ScenePtr GetScene();
      void ResetCamera();
      void ResizeWindow(uint width, uint height) override;

     private:
      SceneRenderPass m_renderPass;
      Light* m_light = nullptr;
    };

    typedef View* ViewRawPtr;
    typedef std::vector<ViewRawPtr> ViewRawPtrArray;

    class PropInspector : public Window
    {
     public:
      enum class ViewType
      {
        Entity,
        Prefab,
        CustomData,
        Component,
        Material,
        Mesh,
        RenderSettings,
        ViewCount
      };

     public:
      explicit PropInspector(XmlNode* node);
      PropInspector();
      virtual ~PropInspector();

      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;
      void SetMaterialView(MaterialPtr mat);
      void SetMeshView(MeshPtr mesh);

     public:
      ViewRawPtrArray m_views;
      ViewType m_activeView = ViewType::Entity;
    };

  } // namespace Editor
} // namespace ToolKit
