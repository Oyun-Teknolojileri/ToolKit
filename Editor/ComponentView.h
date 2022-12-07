#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {

    class ComponentView : public View
    {
     public:
      static void ShowAnimControllerComponent(ParameterVariant* var,
                                              ComponentPtr comp);
      static bool ShowComponentBlock(ComponentPtr& comp,
                                     const bool modifiableComp);
      ComponentView();
      virtual ~ComponentView();
      virtual void Show();
    };

  } // namespace Editor
} // namespace ToolKit