#pragma once

#include "Entity.h"
#include "Framebuffer.h"
#include "Primative.h"
#include "Types.h"

#include <string>

namespace ToolKit
{

  class TK_API Light : public Entity
  {
   public:
    Light();
    virtual ~Light();
    virtual void ParameterEventConstructor();

    EntityType GetType() const override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    // Shadow operations
    virtual void InitShadowMap();
    virtual void UnInitShadowMap();
    FramebufferPtr GetShadowMapFramebuffer();
    RenderTargetPtr GetShadowMapRenderTarget();
    RenderTargetPtr GetShadowMapTempBlurRt();
    MaterialPtr GetShadowMaterial();
    virtual void UpdateShadowCamera();
    virtual float AffectDistance();
    virtual void InitShadowMapDepthMaterial();

   protected:
    void ReInitShadowMap();

   public:
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(bool, CastShadow);
    TKDeclareParam(Vec2, ShadowResolution);
    TKDeclareParam(int, PCFSamples);
    TKDeclareParam(float, PCFRadius);
    TKDeclareParam(float, ShadowThickness);
    TKDeclareParam(float, LightBleedingReduction);

    Mat4 m_shadowMapCameraProjectionViewMatrix;
    float m_shadowMapCameraFar = 1.0f;
    Camera* m_shadowCamera     = nullptr;
    bool m_needToUpdateShadows = false;

   protected:
    bool m_shadowMapInitialized           = false;
    bool m_shadowMapResolutionChanged     = false;
    MaterialPtr m_shadowMapMaterial       = nullptr;
    FramebufferPtr m_depthFramebuffer     = nullptr;
    RenderTargetPtr m_shadowRt            = nullptr;
    RenderTargetPtr m_shadowMapTempBlurRt = nullptr;
  };

  class TK_API DirectionalLight : public Light
  {
   public:
    DirectionalLight();
    virtual ~DirectionalLight();

    EntityType GetType() const override;

    void UpdateShadowFrustum(const EntityRawPtrArray& entities);
    Vec3Array GetShadowFrustumCorners();

   private:
    // Fits the entities into the shadow map camera frustum. As the scene gets
    // bigger, the resolution gets lower.
    void FitEntitiesBBoxIntoShadowFrustum(Camera* lightCamera,
                                          const EntityRawPtrArray& entities);

    // Fits view frustum of the camera into shadow map camera frustum. As the
    // view frustum gets bigger, the resolution gets lower.
    void FitViewFrustumIntoLightFrustum(Camera* lightCamera,
                                        Camera* viewCamera);
  };

  class TK_API PointLight : public Light
  {
   public:
    PointLight();
    virtual ~PointLight();

    EntityType GetType() const override;

    void InitShadowMap() override;
    void UpdateShadowCamera() override;
    float AffectDistance() override;
    void InitShadowMapDepthMaterial() override;

   public:
    TKDeclareParam(float, Radius);
  };

  class TK_API SpotLight : public Light
  {
   public:
    SpotLight();
    virtual ~SpotLight();

    EntityType GetType() const override;
    void UpdateShadowCamera() override;
    float AffectDistance() override;
    void InitShadowMapDepthMaterial() override;

   public:
    TKDeclareParam(float, Radius);
    TKDeclareParam(float, OuterAngle);
    TKDeclareParam(float, InnerAngle);
  };

} // namespace ToolKit
