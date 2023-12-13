/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Sky.h"

namespace ToolKit
{

  class TK_API GradientSky : public SkyBase
  {
   public:
    TKDeclareClass(GradientSky, SkyBase);

    GradientSky();
    virtual ~GradientSky();

    void Init() override;
    MaterialPtr GetSkyboxMaterial() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void GenerateGradientCubemap();
    void GenerateIrradianceCubemap();
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(Vec3, TopColor);
    TKDeclareParam(Vec3, MiddleColor);
    TKDeclareParam(Vec3, BottomColor);
    TKDeclareParam(float, GradientExponent);
    TKDeclareParam(float, IrradianceResolution);
    uint m_size = 1024;

   private:
    bool m_waitingForInit = false;
  };

} // namespace ToolKit