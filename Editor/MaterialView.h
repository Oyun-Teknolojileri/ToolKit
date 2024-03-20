/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "PreviewViewport.h"
#include "View.h"

namespace ToolKit
{
  namespace Editor
  {
    class MaterialView : public View
    {
     public:
      MaterialView();
      ~MaterialView();

      void Show() override;
      void SetMaterials(const MaterialPtrArray& mat);
      void ResetCamera();
      void SetSelectedMaterial(MaterialPtr mat);

     private:
      void UpdatePreviewScene();
      void ShowMaterial(MaterialPtr m_mat);

     private:
      PreviewViewportPtr m_viewport = nullptr;
      MaterialPtrArray m_materials;
      uint m_activeObjectIndx    = 0;
      int m_currentMaterialIndex = 0;
      ScenePtr m_scenes[3];

     public:
      bool m_isTempView = false;
    };

    typedef std::shared_ptr<MaterialView> MaterialViewPtr;

    class TempMaterialWindow : public TempWindow
    {
     public:
      TempMaterialWindow();
      ~TempMaterialWindow();
      void SetMaterial(MaterialPtr mat);
      void OpenWindow();
      void Show() override;

     private:
      MaterialViewPtr m_view;
      bool m_isOpen = true;
    };

    typedef std::shared_ptr<TempMaterialWindow> TempMaterialWindowPtr;

  } // namespace Editor
} // namespace ToolKit