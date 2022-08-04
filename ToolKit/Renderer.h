#pragma once

#include <memory>
#include <unordered_map>

#include "Types.h"
#include "RenderState.h"
#include "Light.h"
#include "Viewport.h"
#include "SpriteSheet.h"
#include "Sky.h"

namespace ToolKit
{
  class Viewport;

  class TK_API Renderer
  {
   public:
    Renderer();
    ~Renderer();

    void RenderScene
    (
      const ScenePtr scene,
      Viewport* viewport,
      const LightRawPtrArray& editor_lights
    );

    void RenderUI(const UILayerPtrArray& uiLayers, Viewport* viewport);

    void Render
    (
      Entity* ntt,
      Camera* cam,
      const LightRawPtrArray& editorLights =
      LightRawPtrArray()
    );

    void SetRenderState(const RenderState* const state, ProgramPtr program);

    void SetRenderTarget
    (
      RenderTarget* renderTarget,
      bool clear = true,
      const Vec4& color = { 0.2f, 0.2f, 0.2f, 1.0f }
    );

    void SwapRenderTarget
    (
      RenderTarget** renderTarget,
      bool clear = true,
      const Vec4& color = { 0.2f, 0.2f, 0.2f, 1.0f }
    );

    void DrawFullQuad(ShaderPtr fragmentShader);
    void DrawCube(Camera* cam, MaterialPtr mat);

   private:
    void RenderEntities
    (
      EntityRawPtrArray& entities,
      Camera* cam,
      Viewport* viewport,
      const LightRawPtrArray& editorLights = LightRawPtrArray()
    );

    /**
    * Removes the entites that are outside of the camera.
    * @param entities All entites.
    * @param camera Camera that is being used for generating frustum.
    */
    void FrustumCull(EntityRawPtrArray& entities, Camera* camera);

    /**
    * Extracts blended entites from given entity array.
    * @param entities Entity array that the transparents will extracted from.
    * @param blendedEntities Entity array that are going to be filled
    * with transparents.
    */
    void GetTransparentEntites
    (
      EntityRawPtrArray& entities,
      EntityRawPtrArray& blendedEntities
    );

    /**
    * Renders the entities immediately. No sorting applied.
    * @param entities All entities to render.
    * @param cam Camera for rendering.
    * @param zoom Zoom amount of camera.
    * @param editorLights All lights.
    */
    void RenderOpaque
    (
      EntityRawPtrArray entities,
      Camera* cam,
      float zoom,
      const LightRawPtrArray& editorLights =
      LightRawPtrArray()
    );

    /**
    * Sorts and renders entities. For double-sided blended entities first
    * render back, than renders front.
    * @param entities All entities to render.
    * @param cam Camera for rendering.
    * @param zoom Zoom amount of camera.
    * @param editorLights All lights.
    */
    void RenderTransparent
    (
      EntityRawPtrArray entities,
      Camera* cam,
      float zoom,
      const LightRawPtrArray& editorLights =
      LightRawPtrArray()
    );

    void RenderSky(Sky* sky, Camera* cam);

    void RenderSkinned(Entity* object, Camera* cam);
    void Render2d(Surface* object, glm::ivec2 screenDimensions);
    void Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions);

    LightRawPtrArray GetBestLights
    (
      Entity* entity,
      const LightRawPtrArray& lights
    );
    void GetEnvironmentLightEntities(EntityRawPtrArray entities);
    void FindEnvironmentLight(Entity* entity, Camera* camera);

    void SetProjectViewModel(Entity* ntt, Camera* cam);
    void BindProgram(ProgramPtr program);
    void LinkProgram(uint program, uint vertexP, uint fragmentP);
    ProgramPtr CreateProgram(ShaderPtr vertex, ShaderPtr fragment);
    void FeedUniforms(ProgramPtr program);
    void FeedLightUniforms(ProgramPtr program);
    void SetVertexLayout(VertexLayout layout);

   public:
    uint m_frameCount = 0;
    uint m_windowWidth = 0;
    uint m_windowHeight = 0;
    Vec4 m_bgColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    MaterialPtr m_overrideMat = nullptr;

    // Grid parameters
    float m_gridCellSize = 0.1f;
    float m_gridSize = 100.0f;
    Vec3 m_gridHorizontalAxisColor = X_AXIS;
    Vec3 m_gridVerticalAxisColor = Z_AXIS;

   private:
    uint m_currentProgram = 0;
    int m_textureIdCount = 0;
    Mat4 m_project;
    Mat4 m_view;
    Mat4 m_model;
    LightRawPtrArray m_lights;
    size_t m_maxLightsPerObject = 12;
    Camera* m_cam = nullptr;
    Material* m_mat = nullptr;
    RenderTarget* m_renderTarget = nullptr;

    std::unordered_map<String, ProgramPtr> m_programs;
    RenderState m_renderState;

    EntityRawPtrArray m_environmentLightEntities;
  };

}  // namespace ToolKit
