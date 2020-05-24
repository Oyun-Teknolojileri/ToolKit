#pragma once

#include "ToolKit.h"

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

		virtual void LookAt(class Camera* cam);
		virtual Billboard* GetCopy() const override;
    virtual void GetCopy(Entity* copyTo) const override;
		virtual EntityType GetType() const override;

	public:
		Settings m_settings;
		Vec3 m_worldLocation;
	};

  class Cube : public Drawable
  {
  public:
    Cube(bool genDef = true);
    Cube(const Vec3& scale);

		virtual Cube* GetCopy() const override;
		virtual void GetCopy(Entity* copyTo) const override;
    virtual EntityType GetType() const override;

	private:
		void Generate(Vec3 scale);
  };

  class Quad : public Drawable
  {
  public:
    Quad(bool genDef = true);

		virtual Quad* GetCopy() const override;
		virtual void GetCopy(Entity* copyTo) const override;
    virtual EntityType GetType() const override;

  private:
    void Generate();
  };

  class Sphere : public Drawable
  {
  public:
    Sphere(bool genDef = true);
    Sphere(float rad);

		virtual Sphere* GetCopy() const override;
		virtual void GetCopy(Entity* copyTo) const override;
    virtual EntityType GetType() const override;

  private:
    void Generate(float rad);
  };

  class Cone : public Drawable
  {
  public:
    Cone(bool genDef = true);
    Cone(float height, float radius, int nSegBase, int nSegHeight);

		virtual Cone* GetCopy() const override;
		virtual void GetCopy(Entity* copyTo) const override;
    virtual EntityType GetType() const override;

  private:
    void Generate(float height, float radius, int nSegBase, int nSegHeight);
  };

  class Arrow2d : public Drawable
  {
  public:
    Arrow2d(bool genDef = true);
    Arrow2d(AxisLabel label); // X - Y - Z.

    virtual Arrow2d* GetCopy() const override;
    virtual void GetCopy(Entity* copyTo) const override;
		virtual EntityType GetType() const override;

	private:
		void Generate();

  private:
    AxisLabel m_label;
  };

  class LineBatch : public Drawable
  {
    LineBatch(); // For copy.

  public:
    LineBatch(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth = 1.0f);

		virtual LineBatch* GetCopy() const override;
		virtual void GetCopy(Entity* copyTo) const override;
		virtual EntityType GetType() const override;
    void Generate(const Vec3Array& linePnts, const Vec3& color, DrawType t, float lineWidth = 1.0f);
  };

}
