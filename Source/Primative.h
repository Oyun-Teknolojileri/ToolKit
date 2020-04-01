#pragma once

#include "Drawable.h"

namespace ToolKit
{

  class Cube : public Drawable
  {
  public:
    Cube();
    ~Cube();
    virtual EntityType GetType() const override;
  };

  class Quad : public Drawable
  {
  public:
    Quad();
    ~Quad();
    virtual EntityType GetType() const override;
  };

  class Sphere : public Drawable
  {
  public:
    Sphere();
    ~Sphere();
    virtual EntityType GetType() const override;
  };

  class Arrow2d : public Drawable
  {
  public:
    enum ArrowType
    {
      X,
      Y,
      Z
    } m_arrowType = X;

  public:
    Arrow2d();
    Arrow2d(ArrowType t);
    ~Arrow2d();
    virtual EntityType GetType() const override;

	private:
		void Generate(ArrowType t);
  };

  class LineBatch : public Drawable
  {
  public:
    LineBatch(const std::vector<glm::vec3>& linePnts, glm::vec3 color, DrawType t);
    ~LineBatch();
		virtual EntityType GetType() const override;
    void Generate(const std::vector<glm::vec3>& linePnts, glm::vec3 color, DrawType t);
  };

}
