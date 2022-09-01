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
    void ParameterEventConstructor();

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    // Shadow operations
    virtual void InitShadowMap();
    virtual void UnInitShadowMap();
    RenderTarget* GetShadowMapRenderTarget();
    MaterialPtr GetShadowMaterial();

   protected:
    virtual void InitShadowMapDepthMaterial();

   public:
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(bool, CastShadow);
    TKDeclareParam(float, ShadowBias);
    TKDeclareParam(Vec2, ShadowResolution);
    TKDeclareParam(int, ShadowPCFKernelSize);

    bool m_isStudioLight = false;
    Mat4 m_shadowMapCameraProjectionViewMatrix;
    float m_shadowMapCameraFar;

   protected:
    bool m_shadowMapInitialized = false;
    bool m_shadowMapResolutionChanged = false;
    MaterialPtr m_shadowMapMaterial = nullptr;
    RenderTarget* m_depthRenderTarget = nullptr;
  };

  class TK_API DirectionalLight : public Light
  {
   public:
     DirectionalLight();
     virtual ~DirectionalLight();

     EntityType GetType() const override;
  };

  class TK_API PointLight : public Light
  {
   public:
    PointLight();
    virtual ~PointLight() {}

    EntityType GetType() const override;

    void InitShadowMap() override;

   protected:
    void InitShadowMapDepthMaterial() override;

   public:
    TKDeclareParam(float, Radius);
  };

  class TK_API SpotLight : public Light
  {
   public:
    SpotLight();
    virtual ~SpotLight() {}

    EntityType GetType() const override;

   protected:
    void InitShadowMapDepthMaterial() override;

   public:
    TKDeclareParam(float, Radius);
    TKDeclareParam(float, OuterAngle);
    TKDeclareParam(float, InnerAngle);
  };

}  // namespace ToolKit
