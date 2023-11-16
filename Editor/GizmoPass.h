/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Gizmo.h"
#include "Pass.h"

namespace ToolKit
{
  namespace Editor
  {

    struct GizmoPassParams
    {
      Viewport* Viewport = nullptr;
      BillboardPtrArray GizmoArray;
    };

    class GizmoPass : public Pass
    {
     public:
      GizmoPass();
      explicit GizmoPass(const GizmoPassParams& params);

      void Render() override;
      void PreRender() override;
      void PostRender() override;

     public:
      GizmoPassParams m_params;

     private:
      SpherePtr m_depthMaskSphere = nullptr;
      CameraPtr m_camera          = nullptr;
    };

    typedef std::shared_ptr<GizmoPass> GizmoPassPtr;

  } // namespace Editor
} // namespace ToolKit