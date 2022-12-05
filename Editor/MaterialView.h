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

     public:
      MaterialPtr m_material;
    };

  } // namespace Editor
} // namespace ToolKit