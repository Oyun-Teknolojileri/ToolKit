#pragma once

#include "EditorViewport.h"
#include "FolderWindow.h"
#include "SceneRenderer.h"
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
      // If isEditable = false;
      //  You should call EndDisabled() before using DropZone & DropSubZone
      static void DropZone(
          uint fallbackIcon,
          const String& file,
          std::function<void(DirectoryEntry& entry)> dropAction,
          const String& dropName = "",
          bool isEditable        = true); 

      static void DropSubZone(
          const String& title,
          uint fallbackIcon,
          const String& file,
          std::function<void(const DirectoryEntry& entry)> dropAction,
          bool isEditable = true);
      static bool IsTextInputFinalized();

      View(const StringView viewName);

      virtual ~View() {}

      virtual void Show() = 0;

     public:
      Entity* m_entity     = nullptr;
      int m_viewID         = 0;
      TexturePtr m_viewIcn = nullptr;
      StringView m_fontIcon;
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
      SceneRendererPtr m_previewRenderer = nullptr;
      Light* m_light                     = nullptr;
     public:
      bool m_isTempView = false;
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
      void SetMaterials(const MaterialPtrArray& mat);
      void SetMeshView(MeshPtr mesh);

     public:
      ViewRawPtrArray m_views;
      ViewType m_activeView = ViewType::Entity;
    };

  } // namespace Editor
} // namespace ToolKit
