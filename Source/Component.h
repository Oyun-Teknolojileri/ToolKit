#pragma once

#include "Types.h"
#include "Serialize.h"

namespace ToolKit
{

  enum class ComponentType
  {
    Component_Base,
    Component_Mesh
  };

  class TK_API Component : public Serializable
  {
  public:
    Component();
    virtual  ~Component();

  public:
    ULongID m_id;
    ComponentType m_type;

  private:
    static ULongID m_handle;
  };

  class TK_API MeshComponent : public Component
  {
  public:
    MeshComponent();
    virtual ~MeshComponent();
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    MeshPtr m_mesh;
    MaterialPtr m_material;
  };

}
