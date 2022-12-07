#pragma once
#include "PropInspector.h"

namespace ToolKit
{
  namespace Editor
  {
    class EditorViewport;
    class MeshView : public View
    {
     public:
      MeshView();
      ~MeshView();
      
      void Show() override;
      void SetMesh(MeshPtr mesh);
      void ResetCamera();

     private:
      PreviewViewport* m_viewport;
      MeshPtr m_mesh;
    };

  } // namespace Editor
} // namespace ToolKit
