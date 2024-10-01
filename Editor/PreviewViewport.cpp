/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "PreviewViewport.h"

#include <DirectionComponent.h>

namespace ToolKit
{
  namespace Editor
  {

    TKDefineClass(PreviewViewport, EditorViewport);

    PreviewViewport::PreviewViewport()
    {
      m_previewRenderer                                 = MakeNewPtr<ForwardSceneRenderPath>();
      m_previewRenderer->m_params.applyGammaTonemapFxaa = true;
      m_previewRenderer->m_params.Cam                   = GetCamera();
      m_previewRenderer->m_params.MainFramebuffer       = m_framebuffer;
    }

    PreviewViewport::~PreviewViewport() { m_previewRenderer = nullptr; }

    void PreviewViewport::Show()
    {
      HandleStates();
      DrawCommands();

      m_previewRenderer->m_params.MainFramebuffer = m_framebuffer;
      GetRenderSystem()->AddRenderTask({[this](Renderer* renderer) -> void { m_previewRenderer->Render(renderer); }});

      // Render color attachment as rounded image
      const FramebufferSettings& fbSettings = m_framebuffer->GetSettings();
      Vec2 imageSize                        = Vec2(fbSettings.width, fbSettings.height);

      Vec2 wndPos                           = ImGui::GetWindowPos();
      Vec2 wndLowerLeft                     = ImGui::GetWindowContentRegionMin();
      Vec2 cursorPos                        = ImGui::GetCursorPos();
      Vec2 currentCursorPos                 = wndLowerLeft + cursorPos + wndPos;

      if (m_isTempView)
      {
        currentCursorPos.y -= 24.0f;
      }

      ImGui::Dummy(imageSize);

      ImGui::GetWindowDrawList()->AddImageRounded(Convert2ImGuiTexture(m_renderTarget),
                                                  currentCursorPos,
                                                  currentCursorPos + imageSize,
                                                  Vec2(0.0f, 0.0f),
                                                  Vec2(1.0f, -1.0f),
                                                  ImGui::GetColorU32(Vec4(1, 1, 1, 1)),
                                                  5.0f);
    }

    ScenePtr PreviewViewport::GetScene() { return m_previewRenderer->m_params.Scene; }

    void PreviewViewport::SetScene(ScenePtr scene)
    {
      scene->Update(0.0f);
      m_previewRenderer->m_params.Scene = scene;
    }

    void PreviewViewport::ResetCamera()
    {
      CameraPtr cam = GetCamera();
      cam->m_node->SetTranslation(Vec3(3.0f, 6.55f, 4.0f) * 0.6f);
      cam->GetComponent<DirectionComponent>()->LookAt(Vec3(0.0f, 1.1f, 0.0f));
    }

    void PreviewViewport::SetViewportSize(uint width, uint height)
    {
      if (width != m_size.x || height != m_size.y)
      {
        m_size = UVec2(width, height);
        OnResizeContentArea((float) width, (float) height);
      }
    }

  } // namespace Editor
} // namespace ToolKit