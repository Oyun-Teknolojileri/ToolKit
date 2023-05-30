/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "EditorViewport.h"
#include "SceneRenderer.h"
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
      void SetScene(ScenePtr scene);
      void ResetCamera();
      void ResizeWindow(uint width, uint height) override;

     private:
      SceneRendererPtr m_previewRenderer = nullptr;

     public:
      bool m_isTempView = false;
    };

    typedef View* ViewRawPtr;
    typedef std::vector<ViewRawPtr> ViewRawPtrArray;

    class PropInspector : public Window
    {
     public:
      explicit PropInspector(XmlNode* node);
      PropInspector();
      virtual ~PropInspector();
      void SetActiveView(ViewType viewType);
      class MaterialView* GetMaterialView();

      void Show() override;
      Type GetType() const override;
      void DispatchSignals() const override;
      void SetMaterials(const MaterialPtrArray& mat);
      void SetMeshView(MeshPtr mesh);

     private:
      void DeterminateSelectedMaterial(Entity* curEntity);

     public:
      ViewRawPtrArray m_views;
      UIntArray m_prefabViews;
      UIntArray m_entityViews;

      ViewType m_activeView = ViewType::Entity;
    };

  } // namespace Editor
} // namespace ToolKit
