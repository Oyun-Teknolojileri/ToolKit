#pragma once

#include "Drawable.h"
#include "RenderState.h"

namespace ToolKit
{

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

		virtual EntityType GetType() const override;
		virtual void LookAt(class Camera* cam);

	public:
		Settings m_settings;
		glm::vec3 m_worldLocation;
	};

  class Cube : public Drawable
  {
  public:
    Cube();

    virtual EntityType GetType() const override;
  };

  class Quad : public Drawable
  {
  public:
    Quad();

    virtual EntityType GetType() const override;
  };

  class Sphere : public Drawable
  {
  public:
    Sphere();

    virtual EntityType GetType() const override;
  };

  class Arrow2d : public Drawable
  {
  public:
    Arrow2d();
    Arrow2d(AxisLabel label); // X - Y - Z.

    virtual EntityType GetType() const override;

	private:
		void Generate();

  private:
    AxisLabel m_label;
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
