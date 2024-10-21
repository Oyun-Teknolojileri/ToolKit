/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Component.h"

namespace ToolKit
{

  typedef std::shared_ptr<class DirectionComponent> DirectionComponentPtr;

  static VariantCategory DirectionComponentCategory {"Direction Component", 10};

  class TK_API DirectionComponent : public Component
  {
   public:
    TKDeclareClass(DirectionComponent, Component);

    DirectionComponent();
    virtual ~DirectionComponent();

    ComponentPtr Copy(EntityPtr ntt) override;

    // Directional functions
    Vec3 GetDirection() const;
    void Pitch(float angle);
    void Yaw(float angle);
    void Roll(float angle);
    void RotateOnUpVector(float angle);
    Vec3 GetUp() const;
    Vec3 GetRight() const;
    void LookAt(const Vec3& target);
    void LookAt(const Vec3& eye, const Vec3& target);

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   private:
    Mat4& GetOwnerWorldTransform() const;

   public:
    mutable bool m_spatialCachesInvalidated = true;

   private:
    mutable Mat4 m_ownerWorldTransformCache;
  };

} //  namespace ToolKit
