/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EditorViewport.h"

namespace ToolKit
{
  namespace Editor
  {

    class PreviewViewport : public EditorViewport
    {
     public:
      TKDeclareClass(PreviewViewport, EditorViewport);

      PreviewViewport();
      ~PreviewViewport();
      void Show() override;
      ScenePtr GetScene();
      void SetScene(ScenePtr scene);
      void ResetCamera();
      void SetViewportSize(uint width, uint height);

     private:
      SceneRenderPathPtr m_previewRenderer = nullptr;

     public:
      bool m_isTempView = false;
    };

    typedef std::shared_ptr<PreviewViewport> PreviewViewportPtr;

  } // namespace Editor
} // namespace ToolKit