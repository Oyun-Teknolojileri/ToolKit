#pragma once

#include <memory>

#include "Component.h"

namespace ToolKit
{

  typedef std::shared_ptr<class DirectionComponent> DirectionComponentPtr;

  static VariantCategory DirectionComponentCategory
  {
    "Direction Component",
    10
  };

  class TK_API DirectionComponent: public Component
  {
   public:
    TKComponentType(DirectionComponent);

    DirectionComponent();
    explicit DirectionComponent(Entity* entity);
    virtual ~DirectionComponent();

    ComponentPtr Copy(Entity* ntt) override;

    // Directional functions
    Vec3 GetDirection();
    void Pitch(float angle);
    void Yaw(float angle);
    void Roll(float angle);
    void RotateOnUpVector(float angle);
    Vec3 GetUp() const;
    Vec3 GetRight() const;
    void LookAt(Vec3 target);
  };

}  //  namespace ToolKit
