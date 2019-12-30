#pragma once

#include "ToolKit.h"
#include "Drawable.h"
#include "MathUtil.h"
#include "Resource.h"
#include <vector>

namespace ToolKit
{

  class Vertex;

  class Surface : public Drawable, public Resource
  {
  public:
    Surface(std::shared_ptr<Texture> texture, glm::vec2 pivotOffset);
    Surface(std::shared_ptr<Texture> texture, const std::vector<Vertex>& vertices);
    Surface(std::string file, glm::vec2 pivotOffset);
    ~Surface();

    EntityType GetType();
    void Load();
    void Init(bool flushClientSideArray = true);

  private:
    void CreateQuat();

  private:
    glm::vec2 m_pivotOffset;
  };

}