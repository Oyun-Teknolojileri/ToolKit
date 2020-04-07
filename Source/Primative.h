#pragma once

#include "Drawable.h"
#include "RenderState.h"

namespace ToolKit
{
  class Camera;

	class Billboard : public Drawable
	{
	public:
		struct Settings
		{
			bool lookAtCamera = true;
      // If grater then 0, place the billboard always at approximately (Difference due to RadialToPlanarDistance conversion) given distance to camera.
			float distanceToCamera = 0.0f;
      // If greater then 0, Fov changes due to window height changes doesn't shrink the object.
			float heightInScreenSpace = 0.0f;
		};

	public:
		Billboard(const Settings& settings);
		virtual ~Billboard();
		virtual EntityType GetType() const override;
		virtual void LookAt(Camera* cam);

	public:
		Settings m_settings;
		glm::vec3 m_worldLocation;
	};

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
    enum class ArrowType
    {
      X,
      Y,
      Z
    } m_arrowType = ArrowType::X;

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
