#pragma once

#include <memory>
#include <unordered_map>

#include "Types.h"
#include "RenderState.h"
#include "Light.h"
#include "Viewport.h"
#include "SpriteSheet.h"

namespace ToolKit
{

  class TK_API Renderer
  {
   public:
    Renderer();
    ~Renderer();
    void RenderScene
    (
      const ScenePtr& scene,
      Viewport* viewport,
      LightRawPtrArray editor_lights
    );
    void RenderUI(const UILayerPtrArray& uiLayers, Viewport* viewport);
    void Render
    (
      Entity* ntt,
      Camera* cam,
      const LightRawPtrArray& editorLights =
      LightRawPtrArray()
    );
    void SetRenderState(const RenderState* const state);
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

   private:
    /**
    * Removes the entites that are outside of the camera.
    * @param entities All entites.
    * @param camera Camera that is being used for generating frustum.
    */
    void FrustumCull(EntityRawPtrArray& entities, Camera* camera);

    /**
    * Extracts blended entites from given entity array.
    * @param entities Entity array that the transparents will extracted from.
    * @param blendedEntities Entity array that are going to be filled with transparents.
    */
    void GetTransparentEntites
    (
      EntityRawPtrArray& entities,
      EntityRawPtrArray& blendedEntities
    );

    /**
    * Renders the entites immidately. No sorting applied.
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
    * Sorts and renders entities. For double-sided blended entites first render back, than renders front.
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
    void RenderSkinned(Drawable* object, Camera* cam);
    void Render2d(Surface* object, glm::ivec2 screenDimensions);
    void Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions);

    LightRawPtrArray GetBestLights
    (
      Entity* entity,
      const LightRawPtrArray& lights
    );
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
    float m_gridCellSize = 0.1f, m_gridSize = 100.0f;
    Vec3 m_gridHorizontalAxisColor = Vec3(1.0f, 0.0f, 0.0f);
    Vec3 m_gridVerticalAxisColor = Vec3(0.0f, 0.0f, 1.0f);

   private:
    uint m_currentProgram = 0;
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
  };

}  // namespace ToolKit
