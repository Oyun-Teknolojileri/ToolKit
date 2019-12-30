#pragma once

#include <memory>
#include "RenderState.h"
#include "Resource.h"
#include "ResourceManager.h"

namespace ToolKit
{

  class Texture;
  class CubeMap;
  class Shader;

  class Material : public Resource
  {
  public:
    Material();
    Material(std::string file);
    ~Material();

    void Load();
    void Init(bool flushClientSideArray = true);
    Material* GetCopy();
		RenderState* GetRenderState();

  public:
    std::shared_ptr<CubeMap> m_cubeMap;
    std::shared_ptr<Texture> m_diffuseTexture;
    std::shared_ptr<Shader> m_vertexShader;
    std::shared_ptr<Shader> m_fragmetShader;
    glm::vec3 m_color;

	private:
		RenderState m_renderState;
  };

  class MaterialManager : public ResourceManager<Material>
  {
  public:
    void Init();
  };

}