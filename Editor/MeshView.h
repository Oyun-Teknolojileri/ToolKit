/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "View.h"

namespace ToolKit
{
  namespace Editor
  {

    class MeshView : public View
    {
     public:
      MeshView();
      ~MeshView();

      void Show() override;
      void SetMesh(MeshPtr mesh);
      void ResetCamera();

     private:
      class PreviewViewport* m_viewport = nullptr;
      EntityPtr m_previewEntity         = nullptr;
      MeshPtr m_mesh                    = nullptr;
    };

  } // namespace Editor
} // namespace ToolKit
