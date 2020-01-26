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
    void Translate(glm::vec3 pos);
    void RotateOnUpVector(float val);
    void GetLocalAxis(glm::vec3& dir, glm::vec3& up, glm::vec3& right);
    
    virtual EntityType GetType();
  };

  class Camera : public Directional
  {
  public:
    struct CamData
    {
      glm::vec3 pos;
      glm::vec3 dir;
    };

  public:
    Camera();
    ~Camera();

		void SetLens(float fov, float width, float height);
    void SetLens(float fov, float width, float height, float near, float far);
    void SetLens(float aspect, float left, float right, float bottom, float top, float near, float far);
    glm::mat4 GetViewMatrix();

    CamData GetData();
    virtual EntityType GetType();

  public:
    glm::mat4 m_projection;
  };

  class Light : public Directional
  {
  public:
    struct LightData
    {
      glm::vec3 pos;
      glm::vec3 dir;
      glm::vec3 color;
    };

  public:
    Light();
    virtual ~Light();

    LightData GetData();
    virtual EntityType GetType();

  public:
    glm::vec3 m_color;
  };

}
