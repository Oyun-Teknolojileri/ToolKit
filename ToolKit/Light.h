/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Entity.h"

namespace ToolKit
{

  // Light
  //////////////////////////////////////////

  class TK_API Light : public Entity
  {
   public:
    TKDeclareClass(Light, Entity);

    Light();
    virtual ~Light();

    void NativeConstruct() override;

    // Shadow
    MaterialPtr GetShadowMaterial();
    virtual void UpdateShadowCamera();
    virtual float AffectDistance();
    virtual void InitShadowMapDepthMaterial();

    /**
     * Returns  0 to 3 number that helps to sort lights by type. DirectionalLight: 0, PointLight: 1, SpotLight: 3.
     * Required for fast iterations. IsA still valid option to use but slower if it will be called 10k or more times.
     */
    enum LightType
    {
      Directional,
      Point,
      Spot
    };

    virtual LightType GetLightType() = 0;

    /**
     * Returns the bounding box of VolumeMesh of the light if its not null, otherwise calls Super::GetBoundingBox().
     */
    BoundingBox GetBoundingBox(bool inWorld = false) const override;

   protected:
    void UpdateShadowCameraTransform();
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;

   public:
    TKDeclareParam(Vec3, Color);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(bool, CastShadow);
    TKDeclareParam(float, ShadowRes);
    TKDeclareParam(int, PCFSamples);
    TKDeclareParam(float, PCFRadius);
    TKDeclareParam(float, ShadowBias);
    TKDeclareParam(float, BleedingReduction);

    Mat4 m_shadowMapCameraProjectionViewMatrix;
    float m_shadowMapCameraFar     = 1.0f;
    CameraPtr m_shadowCamera       = nullptr;
    int m_shadowAtlasLayer         = -1;
    Vec2 m_shadowAtlasCoord        = Vec2(-1.0f);
    bool m_shadowResolutionUpdated = false;
    MeshPtr m_volumeMesh           = nullptr;

   protected:
    MaterialPtr m_shadowMapMaterial = nullptr;
  };

  // DirectionalLight
  //////////////////////////////////////////

  class TK_API DirectionalLight : public Light
  {
   public:
    TKDeclareClass(DirectionalLight, Light);

    DirectionalLight();
    virtual ~DirectionalLight();

    void NativeConstruct() override;
    void UpdateShadowFrustum(RenderJobArray& jobs, const CameraPtr cameraView, const BoundingBox& shadowVolume);

    LightType GetLightType() override { return LightType::Directional; }

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   private:
    // Fits the entities into the shadow map camera frustum. As the scene gets
    // bigger, the resolution gets lower.
    void FitEntitiesBBoxIntoShadowFrustum(CameraPtr lightCamera, const RenderJobArray& jobs);

    // Fits view frustum of the camera into shadow map camera frustum. As the
    // view frustum gets bigger, the resolution gets lower.
    void FitViewFrustumIntoLightFrustum(CameraPtr lightCamera, CameraPtr viewCamera, const BoundingBox& shadowVolume);
  };

  typedef std::shared_ptr<DirectionalLight> DirectionalLightPtr;

  // PointLight
  //////////////////////////////////////////

  class TK_API PointLight : public Light
  {
   public:
    TKDeclareClass(PointLight, Light);

    PointLight();
    virtual ~PointLight();

    LightType GetLightType() override { return LightType::Point; }

    BoundingBox GetBoundingBox(bool inWorld = false) const override;

    void UpdateShadowCamera() override;
    float AffectDistance() override;
    void InitShadowMapDepthMaterial() override;

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    void ParameterConstructor() override;

   public:
    TKDeclareParam(float, Radius);

    BoundingSphere m_boundingSphereCache; //!< Bounding volume, updated after call to UpdateShadowCamera().
  };

  typedef std::shared_ptr<PointLight> PointLightLightPtr;

  // SpotLight
  //////////////////////////////////////////

  class TK_API SpotLight : public Light
  {
   public:
    TKDeclareClass(SpotLight, Light);

    SpotLight();
    virtual ~SpotLight();

    void NativeConstruct() override;

    LightType GetLightType() override { return LightType::Spot; }

    void UpdateShadowCamera() override;
    float AffectDistance() override;
    void InitShadowMapDepthMaterial() override;

   protected:
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    void ParameterConstructor() override;

   public:
    TKDeclareParam(float, Radius);
    TKDeclareParam(float, OuterAngle);
    TKDeclareParam(float, InnerAngle);

    Frustum m_frustumCache; //!< Spot frustum, updated after call to UpdateShadowCamera().

    /**
     * Stores world space bounding box that encapsulates the spot frustum.
     * Used to cull against camera frustum.  Frustum vs Frustum would yield more prices results thus more
     * culled lights.
     */
    BoundingBox m_boundingBoxCache;
  };

  typedef std::shared_ptr<SpotLight> SpotLightPtr;

} // namespace ToolKit
