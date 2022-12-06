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

     private:
      PreviewViewport* m_viewport;
    };

  } // namespace Editor
} // namespace ToolKit