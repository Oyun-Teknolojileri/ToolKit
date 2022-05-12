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
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    virtual void Init();
    virtual void EnableGizmo(bool enable);

   public:
    TKDeclareParam(int, LightType);
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);

   protected:
    bool m_initialized = false;
    bool m_gizmoActive = false;
  };

  class TK_API DirectionalLight : public Light
  {
   public:
     DirectionalLight();
     virtual ~DirectionalLight() {}

    ToolKit::LightType GetLightType() const override;
  };

  class TK_API PointLight : public Light
  {
   public:
    PointLight();
    virtual ~PointLight() {}

    ToolKit::LightType GetLightType() const override;

   public:
    TKDeclareParam(float, Radius);
  };

  class TK_API SpotLight : public Light
  {
   public:
    SpotLight();
    virtual ~SpotLight() {}

    ToolKit::LightType GetLightType() const override;

   public:
    TKDeclareParam(float, Radius);
    TKDeclareParam(float, OuterAngle);
    TKDeclareParam(float, InnerAngle);
  };

}  // namespace ToolKit
