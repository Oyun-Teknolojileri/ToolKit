#pragma once

#include "Directional.h"
#include "Component.h"

namespace ToolKit
{

  namespace Editor
  {

    class EditorCamera : public Camera
    {
    public:
      EditorCamera();
      virtual ~EditorCamera();
      virtual bool IsDrawable() const override;

    private:
      void GenerateFrustum();
    };

  }

}