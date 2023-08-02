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

#include "Gizmo.h"

namespace ToolKit
{
  namespace Editor
  {

    using AnchorPtr       = std::shared_ptr<class Anchor>;
    using AnchorHandlePtr = std::shared_ptr<class AnchorHandle>;

    class AnchorHandle
    {
     public:
      enum class SolidType
      {
        Quad,
        Circle
      };

      struct Params
      {
        // Worldspace data.
        Vec3 worldLoc;
        Vec3 grabPnt;
        // Billboard values.
        Vec3 scale;
        Vec3 translate;
        float angle = 0.f;
        // Geometry.
        DirectionLabel direction;
        Vec3 color;
        SolidType type;
      };

     public:
      AnchorHandle();

      virtual void Generate(const Params& params);
      virtual bool HitTest(const Ray& ray, float& t) const;
      Mat4 GetTransform() const;

     public:
      Params m_params;
      MeshPtr m_mesh;
    };

    class Anchor : public EditorBillboardBase
    {
     public:
      Anchor();
      BillboardType GetBillboardType() const override;

      virtual DirectionLabel HitTest(const Ray& ray) const;
      virtual void Update(float deltaTime);
      bool IsGrabbed(DirectionLabel direction) const;
      void Grab(DirectionLabel direction);
      DirectionLabel GetGrabbedDirection() const;

     protected:
      virtual AnchorHandle::Params GetParam() const;

     public:
      Vec3 m_grabPoint;
      DirectionLabel m_lastHovered;
      std::vector<AnchorHandlePtr> m_handles;

     protected:
      DirectionLabel m_grabbedDirection;
    };

  } // namespace Editor
} // namespace ToolKit
