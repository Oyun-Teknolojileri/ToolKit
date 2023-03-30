#pragma once

#include "Component.h"

namespace ToolKit
{

  typedef std::shared_ptr<class EnvironmentComponent> EnvironmentComponentPtr;
  typedef std::vector<EnvironmentComponentPtr> EnvironmentComponentPtrArray;
  static VariantCategory EnvironmentComponentCategory {"Environment Component",
                                                       90};

  class TK_API EnvironmentComponent : public Component
  {
   public:
    TKComponentType(EnvironmentComponent);

    EnvironmentComponent();
    virtual ~EnvironmentComponent();

    ComponentPtr Copy(Entity* ntt) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    BoundingBox GetBBox();

    void Init(bool flushClientSideArray);

   private:
    void ParameterConstructor();
    void ParameterEventConstructor();
    void ReInitHdri(HdriPtr hdri, float exposure);

   public:
    TKDeclareParam(HdriPtr, Hdri);
    TKDeclareParam(Vec3, Size);
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(float, Exposure);
    TKDeclareParam(MultiChoiceVariant, IBLTextureSize);
  };

} // namespace ToolKit