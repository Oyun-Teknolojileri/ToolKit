#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class EditorViewport;
    class MaterialView : public View
    {
     public:
      MaterialView();

      void Show() override;
      void SetMaterial(MaterialPtr mat);
      void ResetCamera();

     private:
      PreviewViewport* m_viewport;
      MaterialPtr m_mat;
      uint m_activeObjectIndx = 0;
      bool isMeshChanged      = true;
      void updatePreviewScene();
    };

  } // namespace Editor
} // namespace ToolKit