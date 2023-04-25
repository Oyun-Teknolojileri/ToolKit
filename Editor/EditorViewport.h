#pragma once

#include "Global.h"
#include "ToolKit.h"
#include "UI.h"
#include "Viewport.h"

#include <functional>
#include <vector>

namespace ToolKit
{
  class Camera;
  class RenderTarget;

  namespace Editor
  {
    class DirectoryEntry;

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

  } // namespace Editor
} // namespace ToolKit
