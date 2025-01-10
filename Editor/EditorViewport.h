/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "EditorScene.h"
#include "FolderWindow.h"

#include <Viewport.h>

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
      TKDeclareClass(EditorViewport, Window);

      EditorViewport();
      virtual ~EditorViewport();

      virtual void Init(Vec2 size);

      // Window Overrides.
      void Show() override;
      void Update(float deltaTime) override;
      bool IsViewportQueriable() const;
      void DispatchSignals() const override;

      // Viewport overrides.
      void OnResizeContentArea(float width, float height) override;
      virtual void ResizeWindow(uint width, uint height);

      // Editor functions
      void GetContentAreaScreenCoordinates(Vec2* min, Vec2* max) const;
      void SetCamera(CameraPtr cam) override;
      virtual void ResetCameraToDefault(); //!< Reset camera settings to default.

     protected:
      XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
      XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

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
                        EntityPtr* dwMesh,
                        LineBatchPtr* boundingBox,
                        EditorScenePtr currScene);

      Vec3 CalculateDragMeshPosition(bool& meshLoaded,
                                     EditorScenePtr currScene,
                                     EntityPtr dwMesh,
                                     LineBatchPtr* boundingBox);

      void HandleDropMesh(bool& meshLoaded,
                          bool& meshAddedToScene,
                          EditorScenePtr currScene,
                          EntityPtr* dwMesh,
                          LineBatchPtr* boundingBox);

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

      EditorRendererPtr m_editorRenderer;

     protected:
      Vec2 m_contentAreaMin;
      Vec2 m_contentAreaMax;
      IVec2 m_mousePosBegin;
      bool m_needsResize          = false;
      bool m_mouseOverContentArea = false;

     private:
      // States.
      bool m_relMouseModBegin = true;
    };

    typedef std::shared_ptr<EditorViewport> EditorViewportPtr;
    typedef std::vector<EditorViewportPtr> EditorViewportPtrArray;
    typedef std::vector<EditorViewport*> EditorViewportRawPtrArray;

  } // namespace Editor
} // namespace ToolKit
