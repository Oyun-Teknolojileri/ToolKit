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

#include "EditorScene.h"
#include "FolderWindow.h"
#include "Viewport.h"

namespace ToolKit
{
  namespace Editor
  {

    enum class CameraAlignment
    {
      Free,
      Top,
      Front,
      Left,
      User
    };

    class EditorViewport : public Viewport, public Window
    {
     public:
      explicit EditorViewport(XmlNode* node);
      explicit EditorViewport(const Vec2& size);
      EditorViewport(float width, float height);
      virtual ~EditorViewport();

      // Window Overrides.
      void Show() override;
      Type GetType() const override;
      void Update(float deltaTime) override;
      bool IsViewportQueriable() const;
      void DispatchSignals() const override;

      // Viewport overrides.
      void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
      void OnResizeContentArea(float width, float height) override;
      virtual void ResizeWindow(uint width, uint height);

      // Editor functions
      void GetContentAreaScreenCoordinates(Vec2* min, Vec2* max) const;
      void SetCamera(Camera* cam) override;

     protected:
      RenderTargetSettigs GetRenderTargetSettings() override;

      virtual void UpdateContentArea();
      virtual void UpdateWindow();
      virtual void DrawCommands();
      virtual void HandleDrop();
      virtual void DrawOverlays();
      virtual void ComitResize();
      virtual void UpdateSnaps();

      // Mods.
      void FpsNavigationMod(float deltaTime);
      void OrbitPanMod(float deltaTime);
      void AdjustZoom(float delta) override;

     private:
      void LoadDragMesh(bool& meshLoaded,
                        DirectoryEntry dragEntry,
                        Entity** dwMesh,
                        LineBatch** boundingBox,
                        EditorScenePtr currScene);

      Vec3 CalculateDragMeshPosition(bool& meshLoaded,
                                     EditorScenePtr currScene,
                                     Entity* dwMesh,
                                     LineBatch** boundingBox);

      void HandleDropMesh(bool& meshLoaded,
                          bool& meshAddedToScene,
                          EditorScenePtr currScene,
                          Entity** dwMesh,
                          LineBatch** boundingBox);

     public:
      // Window properties.
      static std::vector<class OverlayUI*> m_overlays;
      bool m_mouseOverOverlay           = false;
      CameraAlignment m_cameraAlignment = CameraAlignment::Free;
      int m_additionalWindowFlags       = 0;
      bool m_orbitLock                  = false;
      Vec3 m_snapDeltas; // X: Translation, Y: Rotation, Z: Scale

      // UI Draw commands.
      std::vector<std::function<void(ImDrawList*)>> m_drawCommands;

     protected:
      Vec2 m_contentAreaMin;
      Vec2 m_contentAreaMax;
      IVec2 m_mousePosBegin;
      bool m_needsResize = false;

     private:
      // States.
      bool m_relMouseModBegin = true;
    };

    typedef std::vector<EditorViewport*> EditorViewportRawPtrArray;

  } // namespace Editor
} // namespace ToolKit
