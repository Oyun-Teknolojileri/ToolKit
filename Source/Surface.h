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

    virtual EntityType GetType() const override;
    virtual void Load() override;
    virtual void Init(bool flushClientSideArray = true) override;
		virtual void UnInit() override;

  private:
    void CreateQuat();

  private:
    glm::vec2 m_pivotOffset;
  };

}