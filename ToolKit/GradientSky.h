#pragma once

#include "Sky.h"

namespace ToolKit
{

  class TK_API GradientSky : public SkyBase
  {
   public:
    GradientSky();
    virtual ~GradientSky();

    EntityType GetType() const override;
    void Init() override;
    MaterialPtr GetSkyboxMaterial() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void GenerateGradientCubemap();
    void GenerateIrradianceCubemap();

   public:
    TKDeclareParam(Vec3, TopColor);
    TKDeclareParam(Vec3, MiddleColor);
    TKDeclareParam(Vec3, BottomColor);
    TKDeclareParam(float, GradientExponent);
    TKDeclareParam(float, IrradianceResolution);
    uint m_size = 1024;

   private:
    bool m_onInit              = false;
  };

} // namespace ToolKit