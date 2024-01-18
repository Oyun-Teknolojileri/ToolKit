/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Entity.h"

namespace ToolKit
{
  static VariantCategory CameraCategory {"Camera", 100};

  class TK_API Camera : public Entity
  {
   public:
    TKDeclareClass(Camera, Entity);

    Camera();
    virtual ~Camera();

    void NativeConstruct() override;

    void SetLens(float fov, float aspect);
    void SetLens(float fov, float aspect, float near, float far);
    void SetLens(float left, float right, float bottom, float top, float near, float far);

    inline Mat4 GetViewMatrix() const
    {
      Mat4 view = m_node->GetTransform();
      return glm::inverse(view);
    }

    inline const Mat4& GetProjectionMatrix() const { return m_projection; }

    bool IsOrtographic() const;

    // Tight fit camera frustum to a bounding box with a margin
    void FocusToBoundingBox(const BoundingBox& bb, float margin);

    Vec3Array ExtractFrustumCorner();

    inline float Fov() const { return m_fov; }

    inline float Aspect() const { return m_aspect; }

    inline float Near() const { return m_near; }

    inline float Far() const { return m_far; }

    inline float Left() const { return m_left; }

    inline float Right() const { return m_right; }

    inline float Top() const { return m_top; }

    inline float Bottom() const { return m_bottom; }

    inline Vec3 Position() const { return m_node->GetTranslation(); }

    Vec3 Direction() const;

   protected:
    Entity* CopyTo(Entity* copyTo) const override;
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    XmlNode* DeSerializeImpV045(const SerializationFileInfo& info, XmlNode* parent);

   public:
    /**
     * Used to apply zoom by adjusting camera frustum. The bigger the frustum,
     * the closer the image will be.
     */
    float m_orthographicScale = 0.001f;

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
