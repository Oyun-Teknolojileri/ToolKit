#pragma once

#include "Light.h"

namespace ToolKit
{
  class TK_API Sky : public Entity
  {
   public:
    Sky();
    virtual ~Sky();

    EntityType GetType() const override;

    void Init();
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    MaterialPtr GetSkyboxMaterial();

   private:
    void ParameterConstructor();
    void ParameterEventConstructor();

   public:
    TKDeclareParam(bool, DrawSky);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(float, Exposure);
    TKDeclareParam(HdriPtr, Hdri);

   private:
    bool m_initialized = false;
    MaterialPtr m_skyboxMaterial = nullptr;
  };

}  // namespace ToolKit
