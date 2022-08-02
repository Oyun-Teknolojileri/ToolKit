#include "Component.h"
#include "ResourceComponent.h"
#include "DirectionComponent.h"
#include "ToolKit.h"

namespace ToolKit
{

  Component::Component()
  {
    m_id = GetHandleManager()->GetNextHandle();
  }

  Component::~Component()
  {
  }

  ComponentType Component::GetType() const
  {
    return ComponentType::Base;
  }

  void Component::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* componentNode = CreateXmlNode(doc, XmlComponent, parent);
    WriteAttr
    (
      componentNode, doc, XmlParamterTypeAttr,
      std::to_string
      (
        static_cast<int> (GetType())
      )
    );

    m_localData.Serialize(doc, componentNode);
  }

  void Component::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    m_localData.DeSerialize(doc, parent);
  }

  Component* Component::CreateByType(ComponentType t)
  {
    switch (t)
    {
      case ComponentType::MeshComponent:
        return new MeshComponent();
      break;
      case ComponentType::DirectionComponent:
        return new DirectionComponent();
      break;
      case ComponentType::MaterialComponent:
        return new MaterialComponent();
      break;
      case ComponentType::EnvironmentComponent:
        return new EnvironmentComponent();
      break;
      case ComponentType::Base:
      default:
        assert(false && "Unsupported component type");
      break;
    }
    return nullptr;
  }

}  // namespace ToolKit
