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
      void SetMaterial(MaterialPtr mat);
      void ResetCamera();

     private:
      void UpdatePreviewScene();

     private:
      PreviewViewport* m_viewport = nullptr;
      MaterialPtr m_mat           = nullptr;
      uint m_activeObjectIndx     = 0;
      bool m_isMeshChanged        = true;
    };

  } // namespace Editor
} // namespace ToolKit