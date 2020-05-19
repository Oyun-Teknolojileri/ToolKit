#pragma once

#include "ToolKit.h"
#include "Entity.h"

namespace ToolKit
{
  class Directional : public Entity
  {
  public:
    Directional();
    virtual ~Directional();

    void Pitch(float angle);
    void Yaw(float angle);
    void Roll(float angle);
    void Translate(Vec3 pos);
    void RotateOnUpVector(float angle);
    void GetLocalAxis(Vec3& dir, Vec3& up, Vec3& right) const;
    Vec3 GetDir() const;
    Vec3 GetUp() const;
    Vec3 GetRight() const;
    
    virtual EntityType GetType() const override;
  };

  class Camera : public Directional
  {
  public:
    struct CamData
    {
      Vec3 pos;
      Vec3 dir;
      Mat4 projection;
      float fov;
      float aspect;
      float nearDist;
      float height;
      bool ortographic;
    };

  public:
    Camera();
    ~Camera();

		void SetLens(float fov, float width, float height);
    void SetLens(float fov, float width, float height, float near, float far);
    void SetLens(float aspect, float left, float right, float bottom, float top, float near, float far);
    Mat4 GetViewMatrix() const;

    CamData GetData() const;
    virtual EntityType GetType() const override;

  private:
		float m_fov;
    float m_aspect;
    float m_near;
    float m_height;
		bool m_ortographic;
    Mat4 m_projection;
  };

  class Light : public Directional
  {
  public:
    struct LightData
    {
      Vec3 pos;
      Vec3 dir;
      Vec3 color;
      float intensity;
    };

  public:
    Light();
    virtual ~Light();

    LightData GetData() const;
    virtual EntityType GetType() const override;

  public:
    Vec3 m_color;
    float m_intensity;
  };

}
