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
      explicit EditorCamera(const EditorCamera* cam);
      virtual ~EditorCamera();
      bool IsDrawable() const override;
      Entity* Copy() const override;
      Entity* Instantiate() const override;

     private:
      void GenerateFrustum();
    };

  }  // namespace Editor

}  // namespace ToolKit

