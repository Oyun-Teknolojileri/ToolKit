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

    MaterialPtr GetSkyboxMaterial();

   public:
    TKDeclareParam(bool, DrawSky);

   private:
    bool m_initialized = false;
    MaterialPtr m_skyboxMaterial = nullptr;
  };

}  // namespace ToolKit
