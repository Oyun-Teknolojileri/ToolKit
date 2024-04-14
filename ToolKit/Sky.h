/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Light.h"

namespace ToolKit
{

  class TK_API SkyBase : public Entity
  {
   public:
    TKDeclareClass(SkyBase, Entity);

    SkyBase();
    void NativeConstruct() override;

    virtual void Init();
    virtual void ReInit();
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent) override;
    bool IsInitialized();

    virtual MaterialPtr GetSkyboxMaterial();
    virtual CubeMapPtr GetIrradianceMap();
    HdriPtr GetHdri();
    BoundingBox GetBoundingBox(bool inWorld = false) override;

    virtual bool ReadyToRender();

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    void ConstructSkyMaterial(ShaderPtr vertexPrg, ShaderPtr fragPrg);
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(bool, DrawSky);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(MultiChoiceVariant, IBLTextureSize);

   protected:
    bool m_initialized           = false;
    MaterialPtr m_skyboxMaterial = nullptr;
  };

  class TK_API Sky : public SkyBase
  {
   public:
    TKDeclareClass(Sky, SkyBase);

    Sky();
    virtual ~Sky();

    void Init() override;
    MaterialPtr GetSkyboxMaterial() override;

   protected:
    void ParameterConstructor() override;
    void ParameterEventConstructor() override;
    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const override;

   public:
    TKDeclareParam(float, Exposure);
    TKDeclareParam(HdriPtr, Hdri);
  };

} // namespace ToolKit
