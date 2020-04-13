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

    void Pitch(float val);
    void Yaw(float val);
    void Roll(float val);
    void Translate(Vec3 pos);
    void RotateOnUpVector(float val);
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
      glm::mat4 projection;
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
    glm::mat4 GetViewMatrix() const;

    CamData GetData() const;
    virtual EntityType GetType() const override;

  private:
		float m_fov;
    float m_aspect;
    float m_near;
    float m_height;
		bool m_ortographic;
    glm::mat4 m_projection;
  };

  class Light : public Directional
  {
  public:
    struct LightData
    {
      Vec3 pos;
      Vec3 dir;
      Vec3 color;
    };

  public:
    Light();
    virtual ~Light();

    LightData GetData() const;
    virtual EntityType GetType() const override;

  public:
    Vec3 m_color;
  };

}
