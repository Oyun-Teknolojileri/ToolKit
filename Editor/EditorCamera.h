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
      EditorCamera(const EditorCamera* cam);
      virtual ~EditorCamera();
      virtual bool IsDrawable() const override;
      virtual Entity* Copy() const override;
      virtual Entity* Instantiate() const override;

    private:
      void GenerateFrustum();
    };

  }

}
