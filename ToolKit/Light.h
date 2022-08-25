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
    void SetShadowMapResolution(uint width, uint height);
    Vec2 GetShadowMapResolution();
    RenderTarget* GetShadowMapRenderTarget();
    MaterialPtr GetShadowMaterial();
    Camera* GetShadowMapCamera();
    Mat4 GetShadowMapCameraSpaceMatrix();
    virtual void UpdateShadowMapCamera();

   public:
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(bool, CastShadow);
    TKDeclareParam(float, ShadowMinBias);
    TKDeclareParam(float, ShadowMaxBias);
    TKDeclareParam(Vec2, ShadowResolution);

    bool m_isStudioLight = false;

   protected:
    bool m_shadowMapInitialized = false;
    uint m_shadowMapWidth = 1024;
    uint m_shadowMapHeight = 1024;
    bool m_shadowMapResolutionChanged = false;
    MaterialPtr m_shadowMapMaterial = nullptr;
    Camera* m_shadowMapCamera = nullptr;
    RenderTarget* m_depthRenderTarget = nullptr;
  };

  class TK_API DirectionalLight : public Light
  {
   public:
     DirectionalLight();
     virtual ~DirectionalLight();

     EntityType GetType() const override;

     void InitShadowMap() override;
     void UnInitShadowMap() override;
     void UpdateShadowMapCamera() override;
  };

  class TK_API PointLight : public Light
  {
   public:
    PointLight();
    virtual ~PointLight() {}

    EntityType GetType() const override;

    void InitShadowMap() override;

   public:
    TKDeclareParam(float, Radius);
  };

  class TK_API SpotLight : public Light
  {
   public:
    SpotLight();
    virtual ~SpotLight() {}

    EntityType GetType() const override;

    void InitShadowMap() override;

   public:
    TKDeclareParam(float, Radius);
    TKDeclareParam(float, OuterAngle);
    TKDeclareParam(float, InnerAngle);
  };

}  // namespace ToolKit
