#pragma once

#include <string>

#include "ToolKit.h"
#include "Entity.h"
#include "Types.h"
#include "Primative.h"


namespace ToolKit
{
  class Billboard;

  enum class LightType
  {
    LightBase,
    LightPoint,
    LightDirectional,
    LightSpot
  };

  struct LightData
  {
    int type = -1;
    Vec3 color = ZERO;
    float intensity = -1.0f;
    float radius = -1.0f;
    float outerAngle = -1.0f;
    float innerAngle = -1.0f;
  };

  class TK_API Light : public Entity
  {
   public:
    Light();
    virtual ~Light();

    EntityType GetType() const override;
    virtual LightType GetLightType() const;
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent);

   public:
    LightData m_lightData;
  };

  class TK_API DirectionalLight : public Light
  {
   public:
     DirectionalLight();
     virtual ~DirectionalLight() {}

     // Directional functions
     LightType GetLightType() const override;
     Vec3 GetDirection();
     void Pitch(float angle);
     void Yaw(float angle);
     void Roll(float angle);
     void RotateOnUpVector(float angle);
     Vec3 GetUp() const;
     Vec3 GetRight() const;
     void LookAt(Vec3 target);
  };

  class TK_API PointLight : public Light
  {
   public:
    PointLight();
    virtual ~PointLight() {}

    LightType GetLightType() const override;
  };

  class TK_API SpotLight : public Light
  {
   public:
    SpotLight();
    virtual ~SpotLight() {}

    // Directional functions
    LightType GetLightType() const override;
    Vec3 GetDirection();
    void Pitch(float angle);
    void Yaw(float angle);
    void Roll(float angle);
    void RotateOnUpVector(float angle);
    Vec3 GetUp() const;
    Vec3 GetRight() const;
    void LookAt(Vec3 target);
  };

}  // namespace ToolKit
