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

  class Component : public Serializable
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

  class MeshComponent : public Component
  {
  public:
    virtual void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    virtual void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

  public:
    MeshPtr m_mesh;
    MaterialPtr m_material;
  };

}
