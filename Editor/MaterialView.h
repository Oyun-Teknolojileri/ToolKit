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

      void Show() override;
      void SetMaterial(MaterialPtr mat);

     private:
      ScenePtr m_previewScene;
    };

  } // namespace Editor
} // namespace ToolKit