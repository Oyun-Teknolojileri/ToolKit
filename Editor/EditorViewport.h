#pragma once

#include "ToolKit.h"
#include "Viewport.h"
#include "UI.h"
#include "GlobalDef.h"
#include <functional>

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
      EditorViewport(XmlNode* node);
      EditorViewport(float width, float height);
      virtual ~EditorViewport();

      // Window Overrides.
      virtual void Show() override;
      virtual Type GetType() const override;
      virtual void Update(float deltaTime) override;
      bool IsViewportQueriable() const;
      virtual void DispatchSignals() const override;

      // Viewport overrides.
      virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
      virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
      virtual void OnResize(float width, float height) override;

      // Editor functions
      virtual void Render(class App* app);
      virtual void GetContentAreaScreenCoordinates(Vec2& min, Vec2& max) const;
      virtual Camera* GetCamera() const override;
      virtual void SetCamera(Camera* cam) override;

    protected:
      void UpdateContentArea();
      void UpdateWindow();
      void DrawCommands();
      void HandleDrop();
      void DrawOverlays();

      // Mods.
      void FpsNavigationMode(float deltaTime);
      void OrbitPanMod(float deltaTime);
      virtual void AdjustZoom(float delta) override;

    private:
      void LoadDragMesh
      (
        bool& meshLoaded, 
        DirectoryEntry dragEntry,
        ImGuiIO io,
        Drawable** dwMesh,
        Drawable** boundingBox,
        EditorScenePtr currScene
      );
      Vec3 CalculateDragMeshPosition
      (
        bool& meshLoaded,
        EditorScenePtr currScene,
        Drawable* dwMesh,
        Drawable** boundingBox
      );
      void HandleDropMesh
      (
        bool& meshLoaded,
        bool& meshAddedToScene, 
        EditorScenePtr currScene,
        Drawable** dwMesh, 
        Drawable** boundingBox
      );

    public:
      // Window properties.
      static std::vector<class OverlayUI*> m_overlays;
      bool m_mouseOverOverlay = false;
      CameraAlignment m_cameraAlignment = CameraAlignment::Free;
      int m_additionalWindowFlags = 0;
      bool m_orbitLock = false;

      // UI Draw commands.
      std::vector<std::function<void(ImDrawList*)>> m_drawCommands;

    protected:
      Vec2 m_contentAreaMin;
      Vec2 m_contentAreaMax;

    private:
      // States.
      bool m_relMouseModBegin = true;
    };

  }
}
