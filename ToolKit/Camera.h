#pragma once

#include "Types.h"
#include "Entity.h"

namespace ToolKit
{
  class TK_API Camera : public Entity
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
    explicit Camera(XmlNode* node);
    virtual ~Camera();

    void SetLens(float fov, float width, float height);
    void SetLens(float fov, float width, float height, float near, float far);
    void SetLens
    (
      float left,
      float right,
      float bottom,
      float top,
      float near,
      float far
    );
    Mat4 GetViewMatrix() const;
    Mat4 GetProjectionMatrix() const;
    void SetProjectionMatrix(Mat4 proj);
    bool IsOrtographic() const;

    CamData GetData() const;
    EntityType GetType() const override;

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    void ParameterConstructor();

   public:
    TKDeclareParam(float, Fov);
    TKDeclareParam(float, Aspect);
    TKDeclareParam(float, Near);
    TKDeclareParam(float, Far);
    TKDeclareParam(float, Height);
    TKDeclareParam(float, Left);
    TKDeclareParam(float, Right);
    TKDeclareParam(float, Bottom);
    TKDeclareParam(float, Top);
    TKDeclareParam(bool, Orthographic);

   private:
    float m_fov = 1.0f;
    float m_aspect = 1.0f;
    float m_near = 0.01f;
    float m_far = 1000.0f;
    float m_height = 400.0f;
    float m_left = 10.0f;
    float m_right = 10.0f;
    float m_bottom = 10.0f;
    float m_top = 10.0f;
    bool m_ortographic = false;
    Mat4 m_projection;
  };
}  // namespace ToolKit
