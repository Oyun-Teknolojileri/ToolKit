/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EditorViewport.h"
#include "SceneRenderPath.h"
#include "UI.h"

namespace ToolKit
{

  namespace Editor
  {
    enum class ViewType
    {
      Entity,
      CustomData,
      Component,
      Material,
      Mesh,
      Prefab,
      ViewCount
    };

    class View
    {
     public:
      // If isEditable = false;
      //  You should call EndDisabled() before using DropZone & DropSubZone
      static void DropZone(uint fallbackIcon,
                           const String& file,
                           std::function<void(DirectoryEntry& entry)> dropAction,
                           const String& dropName = "",
                           bool isEditable        = true);

      static void DropSubZone(const String& title,
                              uint fallbackIcon,
                              const String& file,
                              std::function<void(const DirectoryEntry& entry)> dropAction,
                              bool isEditable = true);
      static bool IsTextInputFinalized();

      View(const StringView viewName);
      virtual ~View();

      virtual void Show() = 0;

     public:
      EntityWeakPtr m_entity;
      int m_viewID         = 0;
      TexturePtr m_viewIcn = nullptr;
      StringView m_fontIcon;
      const StringView m_viewName;
    };

    class PreviewViewport : public EditorViewport
    {
     public:
      PreviewViewport();
      ~PreviewViewport();
      void Show() override;
      ScenePtr GetScene();
      void SetScene(ScenePtr scene);
      void ResetCamera();
      void SetViewportSize(uint width, uint height);

     private:
      SceneRenderPathPtr m_previewRenderer = nullptr;

     public:
      bool m_isTempView = false;
    };

    typedef View* ViewRawPtr;
    typedef std::vector<ViewRawPtr> ViewRawPtrArray;

    class PropInspector : public Window
    {
     public:
      PropInspector();
      explicit PropInspector(XmlNode* node);
      virtual ~PropInspector();
      void SetActiveView(ViewType viewType);
      class MaterialView* GetMaterialView();

      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;
      void SetMaterials(const MaterialPtrArray& mat);
      void SetMeshView(MeshPtr mesh);

     private:
      void DeterminateSelectedMaterial(EntityPtr curEntity);

     public:
      ViewRawPtrArray m_views;
      UIntArray m_prefabViews;
      UIntArray m_entityViews;

      ViewType m_activeView = ViewType::Entity;
    };

  } // namespace Editor
} // namespace ToolKit
