/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Object.h"
#include "ToolKit.h"
#include "Entity.h"



namespace ToolKit
{

  TKDefineClass(Object, Object);

  Object::Object() { _idBeforeCollision = NULL_HANDLE; }

  Object::~Object()
  {
    if (HandleManager* handleMan = GetHandleManager())
    {
      handleMan->ReleaseHandle(GetIdVal());
    }
  }

  void Object::NativeConstruct()
  {
    ParameterConstructor();
    ParameterEventConstructor();
  }

  void Object::NativeDestruct() {}

  void Object::ParameterConstructor()
  {
    ULongID id = GetHandleManager()->GenerateHandle();
    Id_Define(id, EntityCategory.Name, EntityCategory.Priority, true, false);
  }

  void Object::ParameterEventConstructor() {}

  ObjectPtr Object::Copy() const { return nullptr; }

  XmlNode* Object::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    assert(doc != nullptr && parent != nullptr);

    XmlNode* objNode = CreateXmlNode(doc, StaticClass()->Name, parent);
    WriteAttr(objNode, doc, XmlObjectClassAttr, Class()->Name);

    m_localData.Serialize(doc, objNode);
    return objNode;
  }

  XmlNode* Object::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    assert(parent != nullptr && "Root of the object can't be null.");

    ULongID id = GetIdVal();
    GetHandleManager()->ReleaseHandle(id);
    m_localData.DeSerialize(info, parent);
    PreventIdCollision();

    // Construction progress from bottom up.
    return parent;
  }

  void Object::PostDeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    ParameterEventConstructor(); // Set all the events after data deserialized.
  }

  void Object::PreventIdCollision()
  {
    HandleManager* handleMan = GetHandleManager();
    ULongID idInFile         = GetIdVal();

    if (!handleMan->IsHandleUnique(idInFile))
    {
      _idBeforeCollision = idInFile;
      SetIdVal(handleMan->GenerateHandle());
    }
    else
    {
      handleMan->AddHandle(idInFile);
    }
  }

} // namespace ToolKit
