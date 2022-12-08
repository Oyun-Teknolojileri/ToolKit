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
      void GenerateFrustum();

     private:
      void CreateGizmo();
    };

  } // namespace Editor
} // namespace ToolKit
