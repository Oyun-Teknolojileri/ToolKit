#pragma once

#include "Entity.h"
#include "Types.h"

namespace ToolKit
{

  static VariantCategory CameraCategory {"Camera", 100};

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
      float far;
      bool ortographic;
    };

   public:
    Camera();
    virtual ~Camera();

    void SetLens(float fov, float aspect);
    void SetLens(float fov, float aspect, float near, float far);
    void SetLens(float left,
                 float right,
                 float bottom,
                 float top,
                 float near,
                 float far);

    Mat4 GetViewMatrix() const;
    Mat4 GetProjectionMatrix() const;
    bool IsOrtographic() const;

    CamData GetData() const;
    EntityType GetType() const override;

    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    // Tight fit camera frustum to a bounding box with a margin
    void FocusToBoundingBox(const BoundingBox& bb, float margin);

    float Fov() const;
    float Aspect() const;
    float Near() const;
    float Far() const;
    float Left() const;
    float Right() const;
    float Top() const;
    float Bottom() const;
    Vec3 Position() const;
    Vec3 Direction() const;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    void ParameterConstructor();
    void ParameterEventConstructor();

   public:
    /**
     * Used to apply zoom by adjusting camera frustum. The bigger the frustum,
     * the closer the image will be.
     */
    float m_orthographicScale = 1.0f;

    TKDeclareParam(float, Fov);
    TKDeclareParam(float, NearClip);
    TKDeclareParam(float, FarClip);
    TKDeclareParam(bool, Orthographic);
    TKDeclareParam(float, OrthographicScale);

   private:
    float m_fov        = 1.0f;
    float m_aspect     = 1.0f;
    float m_near       = 0.01f;
    float m_far        = 1000.0f;
    float m_left       = 10.0f;
    float m_right      = 10.0f;
    float m_bottom     = 10.0f;
    float m_top        = 10.0f;
    bool m_ortographic = false;
    Mat4 m_projection;
  };
} // namespace ToolKit
