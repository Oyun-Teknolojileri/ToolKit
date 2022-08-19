#pragma once

#include <string>

#include "Entity.h"
#include "Types.h"
#include "Primative.h"

namespace ToolKit
{
  class TK_API Light : public Entity
  {
   public:
    Light();
    virtual ~Light();

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   public:
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);
  };

  class TK_API DirectionalLight : public Light
  {
   public:
     DirectionalLight();
     virtual ~DirectionalLight() {}

     EntityType GetType() const override;
  };

  class TK_API PointLight : public Light
  {
   public:
    PointLight();
    virtual ~PointLight() {}

    EntityType GetType() const override;

   public:
    TKDeclareParam(float, Radius);
  };

  class TK_API SpotLight : public Light
  {
   public:
    SpotLight();
    virtual ~SpotLight() {}

    EntityType GetType() const override;

   public:
    TKDeclareParam(float, Radius);
    TKDeclareParam(float, OuterAngle);
    TKDeclareParam(float, InnerAngle);
  };

}  // namespace ToolKit
