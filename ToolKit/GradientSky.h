/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Renderer.h"
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

    bool ReadyToRender() override;

    /**
     * This functions sets scene name of gradient sky.
     * This function is being called while the Scene is initializing.
     */
    void SetSceneName(const String& sceneName);
    void SaveIBLTexturesToFile();

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void GenerateGradientCubemap(Renderer* renderer);
    void GenerateIrradianceCubemap(Renderer* renderer);
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(Vec3, TopColor);
    TKDeclareParam(Vec3, MiddleColor);
    TKDeclareParam(Vec3, BottomColor);
    TKDeclareParam(float, GradientExponent);

   private:
    bool m_waitingForInit = false;
    FramebufferPtr m_frameBuffer;

    String m_sceneName = "tempName"; //!< This variable is set when a scene inits gradient sky
  };

} // namespace ToolKit