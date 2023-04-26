#pragma once

#include "Camera.h"
#include "ParameterBlock.h"

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
      void PostDeSerialize() override;
      void GenerateFrustum();

     public:
      TKDeclareParam(VariantCallback, Poses);

     private:
      void CreateGizmo();
      void ParameterConstructor();

     private:
      bool m_posessed = false;  
    };

  } // namespace Editor
} // namespace ToolKit
