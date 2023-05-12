#pragma once
#include "PropInspector.h"

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
      void SetSelectedMaterial(MaterialPtr m_mat);

     private:
      void UpdatePreviewScene();
      void ShowMaterial(MaterialPtr m_mat);

     private:
      PreviewViewport* m_viewport  = nullptr;
      MaterialPtrArray m_materials;
      uint m_activeObjectIndx      = 0;
      bool m_isMeshChanged         = true;
      int m_currentMaterialIndex   = 0;
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