#pragma once

#include "Drawable.h"

namespace ToolKit
{

  class Cube : public Drawable
  {
  public:
    Cube();
    ~Cube();
    EntityType GetType();
  };

  class Quad : public Drawable
  {
  public:
    Quad();
    ~Quad();
    EntityType GetType();
  };

  class Sphere : public Drawable
  {
  public:
    Sphere();
    ~Sphere();
    EntityType GetType();
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
    EntityType GetType();

	private:
		void Generate(ArrowType t);
  };

}
