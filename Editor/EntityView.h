#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {

    class EntityView : public View
    {
     public:
      EntityView();
      virtual ~EntityView();
      virtual void Show();
      virtual void ShowParameterBlock();

     protected:
      void ShowAnchorSettings();
    };

  } // namespace Editor
} // namespace ToolKit