/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
    TKDeclareClass(Camera, Entity);

    Camera();
    virtual ~Camera();

    void SetLens(float fov, float aspect);
    void SetLens(float fov, float aspect, float near, float far);
    void SetLens(float left, float right, float bottom, float top, float near, float far);

    Mat4 GetViewMatrix() const;
    Mat4 GetProjectionMatrix() const;
    bool IsOrtographic() const;

    CamData GetData() const;
    EntityType GetType() const override;

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
    void SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerializeImp(XmlDocument* doc, XmlNode* parent) override;

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
