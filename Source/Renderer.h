#pragma once

#include "Types.h"
#include "RenderState.h"
#include <memory>
#include <unordered_map>

namespace ToolKit
{

  class Drawable;
  class Camera;
  class Light;
  class Surface;
  class SpriteAnimation;
  class Shader;
  class Material;
  class Viewport;
  class RenderTarget;
  class Renderer;

  struct RenderTechnique
  {
    std::function<void(Renderer*)> Render;
    int priority = 0;
  };

  struct RenderJob
  {
    Entity* entity;
    RenderTechnique technique;
  };

  class TK_API Renderer
  {
  public:
    Renderer();
    ~Renderer();
    void RenderScene(const ScenePtr& scene, Viewport* viewport, LightRawPtrArray editor_lights);
    void Render(Entity* ntt, Camera* cam, const LightRawPtrArray& lights = LightRawPtrArray());
    void SetRenderState(const RenderState* const state);
    void SetRenderTarget(RenderTarget* renderTarget, bool clear = true, const Vec4& color = { 0.2f, 0.2f, 0.2f, 1.0f });
    void SwapRenderTarget(RenderTarget** renderTarget, bool clear = true, const Vec4& color = { 0.2f, 0.2f, 0.2f, 1.0f });
    void DrawFullQuad(ShaderPtr fragmentShader);

  private:
    void RenderSkinned(Drawable* object, Camera* cam);
    void Render2d(Surface* object, glm::ivec2 screenDimensions);
    void Render2d(SpriteAnimation* object, glm::ivec2 screenDimensions);

    void SetProjectViewModel(Entity* ntt, Camera* cam);
    void BindProgram(ProgramPtr program);
    void LinkProgram(uint program, uint vertexP, uint fragmentP);
    ProgramPtr CreateProgram(ShaderPtr vertex, ShaderPtr fragment);
    void FeedUniforms(ProgramPtr program);
    void SetVertexLayout(VertexLayout layout);

  public:
    uint m_frameCount = 0;
    uint m_windowWidth = 0;
    uint m_windowHeight = 0;
    Vec4 m_bgColor = { 0.2f, 0.2f, 0.2f, 1.0f };
    MaterialPtr m_overrideMat = nullptr;

  private:
    uint m_currentProgram = 0;
    Mat4 m_project;
    Mat4 m_view;
    Mat4 m_model;
    LightRawPtrArray m_lights;
    Camera* m_cam = nullptr;
    Material* m_mat = nullptr;
    RenderTarget* m_renderTarget = nullptr;

    std::unordered_map<String, ProgramPtr> m_programs;
    RenderState m_renderState;
  };

}
