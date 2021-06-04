#pragma once

#include "UI.h"

namespace ToolKit
{
  namespace Editor
  {

    class PropInspector : public Window
    {
    public:
      PropInspector();
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void DispatchSignals() const override;
    };

  }
}