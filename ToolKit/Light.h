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

    // Shadow
    MaterialPtr GetShadowMaterial();
    virtual void UpdateShadowCamera();
    virtual float AffectDistance();
    virtual void InitShadowMapDepthMaterial();

   protected:
    void UpdateShadowCameraTransform();

   public:
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(bool, CastShadow);
    TKDeclareParam(float, ShadowRes);
    TKDeclareParam(int, PCFSamples);
    TKDeclareParam(float, PCFRadius);
    TKDeclareParam(float, ShadowThickness);
    TKDeclareParam(float, LightBleedingReduction);

    Mat4 m_shadowMapCameraProjectionViewMatrix;
    float m_shadowMapCameraFar     = 1.0f;
    Camera* m_shadowCamera         = nullptr;
    int m_shadowAtlasLayer         = -1;
    Vec2 m_shadowAtlasCoord        = Vec2(-1.0f);
    bool m_shadowResolutionUpdated = false;

   protected:
    bool m_shadowMapResolutionChanged = false;
    MaterialPtr m_shadowMapMaterial   = nullptr;
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
