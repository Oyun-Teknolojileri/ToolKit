/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "BillboardPass.h"
#include "EditorLight.h"
#include "Gizmo.h"
#include "GizmoPass.h"
#include "Global.h"
#include "OutlinePass.h"
#include "Pass.h"
#include "PostProcessPass.h"
#include "Primative.h"
#include "RenderSystem.h"
#include "SingleMaterialPass.h"
#include "MobileSceneRenderPath.h"
#include "GammaPass.h"
#include "BloomPass.h"
#include "FxaaPass.h"

namespace ToolKit
{
  namespace Editor
  {

    /**
     * Enumeration for available render modes for the editor.
     */
    enum class EditorLitMode
    {
      /**
       * Uses editor lights which is a three point lighting system attached to
       * camera. Always lit where the viewport camera looks at.
       */
      EditorLit,
      /**
       * Renders scene with albedo colors only.
       */
      Unlit,
      /**
       * Uses lights in the scene with all rendering features enabled.
       */
      FullyLit,
      /**
       * Shows a heat map based on number of lights illuminating the object.
       */
      LightComplexity,
      /**
       * Shows lighting result with a white material assigned to all objects.
       */
      LightingOnly,
      /**
       * Renders scene exactly as in game.
       */
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
      /**
       * Pass parameters.
       */
      EditorRenderParams m_params;

     private:
      /**
       * Three point lighting system which is used to illuminate the scene in
       * EditorLit mode.
       */
      ThreePointLightSystemPtr m_lightSystem            = nullptr;

      /**
       * Override material for EditorLitMode::Unlit.
       */
      MaterialPtr m_unlitOverride                       = nullptr;
      MaterialPtr m_blackMaterial                       = nullptr;

      BillboardPassPtr m_billboardPass                  = nullptr;
      MobileSceneRenderPathPtr m_mscenePass             = nullptr;
      ForwardRenderPassPtr m_uiPass                     = nullptr;
      ForwardRenderPassPtr m_editorPass                 = nullptr;
      GizmoPassPtr m_gizmoPass                          = nullptr;
      TonemapPassPtr m_tonemapPass                      = nullptr;
      GammaPassPtr m_gammaPass                          = nullptr;
      BloomPassPtr m_bloomPass                          = nullptr;
      SSAOPassPtr m_ssaoPass                            = nullptr;
      OutlinePassPtr m_outlinePass                      = nullptr;
      FXAAPassPtr m_fxaaPass                            = nullptr;
      FullQuadPassPtr m_skipFramePass                   = nullptr;
      SingleMatForwardRenderPassPtr m_singleMatRenderer = nullptr;
      CameraPtr m_camera                                = nullptr;

      /**
       * Selected entity list
       */
      EntityPtrArray m_selecteds;
    };

    typedef std::shared_ptr<EditorRenderer> EditorRendererPtr;

  } // namespace Editor
} // namespace ToolKit
