#pragma once

#include <memory>
#include "Types.h"
#include "Serialize.h"

namespace ToolKit
{

#define TKComponentType(type) \
  static ComponentType GetTypeStatic() { return ComponentType::type; } \
  virtual ComponentType GetType() const { return ComponentType::type; }

  enum class ComponentType
  {
    Base,
    MeshComponent
  };

  class TK_API Component : public Serializable
  {
   public:
    Component();
    virtual ~Component();
    virtual ComponentType GetType() const;
    virtual ComponentPtr Copy() = 0;

   public:
    ULongID m_id;

   private:
    static ULongID m_handle;
  };

  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;

  class TK_API MeshComponent : public Component
  {
   public:
    TKComponentType(MeshComponent);

    MeshComponent();
    virtual ~MeshComponent();
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    virtual ComponentPtr Copy();
    BoundingBox GetAABB();

    void Init(bool flushClientSideArray);

   public:
    MeshPtr m_mesh;
    MaterialPtr m_material;
  };

}  // namespace ToolKit
