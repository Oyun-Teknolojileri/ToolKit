/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "BillboardPass.h"
#include "BloomPass.h"
#include "EditorLight.h"
#include "EditorTypes.h"
#include "ForwardSceneRenderPath.h"
#include "GammaTonemapFxaaPass.h"
#include "Gizmo.h"
#include "GizmoPass.h"
#include "OutlinePass.h"
#include "Pass.h"
#include "Primative.h"
#include "RenderSystem.h"

namespace ToolKit
{
  namespace Editor
  {

    /** Enumeration for available render modes for the editor. */
    enum class EditorLitMode
    {
      /**
       * Uses editor lights which is a three point lighting system attached to
       * camera. Always lit where the viewport camera looks at.
       */
      EditorLit,
      /** Uses lights in the scene with all rendering features enabled. */
      FullyLit,
      /** Shows lighting result with a white material assigned to all objects. */
      LightingOnly,
      /** Renders scene exactly as in game. */
      Game
    };

    struct EditorRenderParams
    {
      class App* App                 = nullptr;
      class EditorViewport* Viewport = nullptr;
      EditorLitMode LitMode          = EditorLitMode::EditorLit;
    };

    class EditorRenderer : public RenderPath
    {
     public:
      EditorRenderer();
      explicit EditorRenderer(const EditorRenderParams& params);
      virtual ~EditorRenderer();

      void Render(Renderer* renderer) override;
      void PreRender();
      void PostRender();

     private:
      void SetLitMode(Renderer* renderer, EditorLitMode mode);
      void InitRenderer();
      void OutlineSelecteds(Renderer* renderer);

     public:
      /** Pass parameters. */
      EditorRenderParams m_params;

     private:
      /** Three point lighting system which is used to illuminate the scene in EditorLit mode. */
      ThreePointLightSystemPtr m_lightSystem         = nullptr;

      /** Override material for EditorLitMode::Unlit. */
      MaterialPtr m_unlitOverride                    = nullptr;
      MaterialPtr m_blackMaterial                    = nullptr;

      BillboardPassPtr m_billboardPass               = nullptr;
      SceneRenderPathPtr m_sceneRenderPath           = nullptr;
      ForwardRenderPassPtr m_uiPass                  = nullptr;
      ForwardRenderPassPtr m_editorPass              = nullptr;
      GizmoPassPtr m_gizmoPass                       = nullptr;
      SSAOPassPtr m_ssaoPass                         = nullptr;
      OutlinePassPtr m_outlinePass                   = nullptr;
      FullQuadPassPtr m_skipFramePass                = nullptr;
      GammaTonemapFxaaPassPtr m_gammaTonemapFxaaPass = nullptr;
      CameraPtr m_camera                             = nullptr;

      /** Selected entity list. */
      EntityPtrArray m_selecteds;

      RenderData m_renderData;
      RenderData m_uiRenderData;

      /** Internal render job array for rendering selected entities. */
      RenderJobArray m_unCulledRenderJobs;
    };

    typedef std::shared_ptr<EditorRenderer> EditorRendererPtr;

  } // namespace Editor
} // namespace ToolKit
