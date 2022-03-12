#pragma once

#include "stdafx.h"
#include "Component.h"

namespace ToolKit
{

  ULongID Component::m_handle = NULL_HANDLE;

  Component::Component()
  {
    m_id = ++m_handle;
    m_type = ComponentType::Component_Base;
  }

  Component::~Component()
  {
  }

  MeshComponent::MeshComponent()
  {
    m_type = ComponentType::Component_Mesh;
  }

  MeshComponent::~MeshComponent()
  {
  }

  void MeshComponent::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
  }

  void MeshComponent::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
  }

}
