#pragma once

#include "Camera.h"

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
      Entity* Copy() const override;
      Entity* Instantiate() const override;

     private:
      void GenerateFrustum();
    };

  }  // namespace Editor
}  // namespace ToolKit
