#include "stdafx.h"

#include "ParameterBlock.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  ULongID ParameterVariantBase::m_handle = 0;

  ParameterVariantBase::ParameterVariantBase()
  {
    m_id = ++m_handle;
  }

  ParameterVariant::ParameterVariant()
  {
    *this = 0;
  }

  ParameterVariant::~ParameterVariant()
  {
  }

  ParameterVariant::ParameterVariant(bool var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(byte var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(ubyte var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(float var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(int var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(uint var)
  {
    *this = var;
  }
  ParameterVariant::ParameterVariant(const Vec3& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const Vec4& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const Mat3& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const Mat4& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const String& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const char* var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const ULongID& var)
  {
    *this = var;
  }

  ParameterVariant::VariantType ParameterVariant::GetType() const
  {
    return m_type;
  }

  ParameterVariant& ParameterVariant::operator= (bool var)
  {
    m_type = VariantType::Bool; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (byte var)
  {
    m_type = VariantType::byte; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (ubyte var)
  {
    m_type = VariantType::ubyte; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (float var)
  {
    m_type = VariantType::Float; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (int var)
  {
    m_type = VariantType::Int; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (uint var)
  {
    m_type = VariantType::UInt; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Vec3& var)
  {
    m_type = VariantType::Vec3; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Vec4& var)
  {
    m_type = VariantType::Vec4; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Mat3& var)
  {
    m_type = VariantType::Mat3; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Mat4& var)
  {
    m_type = VariantType::Mat4; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const String& var)
  {
    m_type = VariantType::String; 
    m_var = var;
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const char* var)
  {
    m_type = VariantType::String; 
    m_var = String(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const ULongID& var)
  {
    m_type = VariantType::ULongID; 
    m_var = var;
    return *this;
  }

  void ParameterVariant::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* node = doc->allocate_node(rapidxml::node_element, XmlParamterElement.c_str());
    WriteAttr(node, doc, XmlParamterTypeAttr, std::to_string((int)m_type));
    WriteAttr(node, doc, "name", m_name);
    WriteAttr(node, doc, "category", m_category.Name);
    WriteAttr(node, doc, "priority", std::to_string(m_category.Priority));
    WriteAttr(node, doc, "exposed", std::to_string(m_exposed));
    WriteAttr(node, doc, "editable", std::to_string(m_editable));

    switch (m_type)
    {
    case VariantType::Bool:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<bool>()));
    }
    break;
    case VariantType::byte:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<byte>()));
    }
    break;
    case VariantType::ubyte:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<ubyte>()));
    }
    break;
    case VariantType::Float:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<float>()));
    }
    break;
    case VariantType::Int:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<int>()));
    }
    break;
    case VariantType::UInt:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<uint>()));
    }
    break;
    case VariantType::Vec3:
    {
      WriteVec(node, doc, GetCVar<Vec3>());
    }
    break;
    case VariantType::Vec4:
    {
      WriteVec(node, doc, GetCVar<Vec4>());
    }
    break;
    case VariantType::Mat3:
    {
      const Mat3& val = GetCVar<Mat3>();
      for (int i = 0; i < 3; i++)
      {
        XmlNode* row = CreateXmlNode(doc, "row", node);
        WriteVec(row, doc, glm::row(val, i));
      }
    }
    break;
    case VariantType::Mat4:
    {
      const Mat4& val = GetCVar<Mat4>();
      for (int i = 0; i < 4; i++)
      {
        XmlNode* row = CreateXmlNode(doc, "row", node);
        WriteVec(row, doc, glm::row(val, i));
      }
    }
    break;
    case VariantType::String:
    {
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), GetCVar<String>());
    }
    break;
    case VariantType::ULongID:
      WriteAttr(node, doc, XmlParamterValAttr.c_str(), std::to_string(GetCVar<ULongID>()));
      break;
    default:
      assert(false && "Invalid type.");
      break;
    }

    parent->append_node(node);
  }

  void ParameterVariant::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    if (parent == nullptr)
    {
      assert(false && "Unbound parameter can not exist");
      return;
    }

    XmlAttribute* attr = parent->first_attribute(XmlParamterTypeAttr.c_str());
    m_type = (VariantType)std::atoi(attr->value());
    ReadAttr(parent, "name", m_name);
    ReadAttr(parent, "category", m_category.Name);
    ReadAttr(parent, "priority", m_category.Priority);


    // Custom Parameter Fix. TODO REMOVE AFTER UPDATING EACH SCENE
    if (m_category.Name.empty())
    {
      m_category = CustomDataCategory;
    }

    ReadAttr(parent, "exposed", m_exposed);
    ReadAttr(parent, "editable", m_editable);

    switch (m_type)
    {
    case VariantType::Bool:
    {
      bool val(false);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::byte:
    {
      byte val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::ubyte:
    {
      ubyte val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Float:
    {
      float val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Int:
    {
      int val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::UInt:
    {
      int val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Vec3:
    {
      Vec3 var;
      ReadVec(parent, var);
      m_var = var;
    }
    break;
    case VariantType::Vec4:
    {
      Vec4 var;
      ReadVec(parent, var);
      m_var = var;
    }
    break;
    case VariantType::String:
    {
      String val;
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    case VariantType::Mat3:
    {
      Mat3 val;
      Vec3 vec;
      XmlNode* row = parent->first_node();
      for (int i = 0; i < 3; i++)
      {
        ReadVec<Vec3>(row, vec);
        val = glm::row(val, i, vec);
        row = row->next_sibling();
      }
      m_var = val;
    }
    break;
    case VariantType::Mat4:
    {
      Mat4 val;
      Vec4 vec;
      XmlNode* row = parent->first_node();
      for (int i = 0; i < 4; i++)
      {
        ReadVec<Vec4>(row, vec);
        val = glm::row(val, i, vec);
        row = row->next_sibling();
      }
      m_var = val;
    }
    break;
    case VariantType::ULongID:
    {
      ULongID val(0);
      ReadAttr(parent, XmlParamterValAttr, val);
      m_var = val;
    }
    break;
    default:
      assert(false && "Invalid type.");
      break;
    }
  }

  void ParameterBlock::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* blockNode = doc->allocate_node(rapidxml::node_element, XmlParamBlockElement.c_str());
    for (const ParameterVariant& var : m_variants)
    {
      var.Serialize(doc, blockNode);
    }
    parent->append_node(blockNode);
  }

  void ParameterBlock::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    m_variants.clear();

    if (XmlNode* block = parent->first_node(XmlParamBlockElement.c_str()))
    {
      XmlNode* param = block->first_node(XmlParamterElement.c_str());
      while (param != nullptr)
      {
        ParameterVariant var;
        var.DeSerialize(doc, param);
        Add(var);
        param = param->next_sibling();
      }
    }
  }

  ParameterVariant& ParameterBlock::operator[](size_t index)
  {
    return m_variants[index];
  }

  const ParameterVariant& ParameterBlock::operator[](size_t index) const
  {
    return m_variants[index];
  }

  void ParameterBlock::Add(const ParameterVariant& var) 
  { 
    m_variants.push_back(var);
  }

  void ParameterBlock::Remove(ULongID id)
  {
    for (size_t i = 0; i < m_variants.size(); i++)
    {
      if (m_variants[i].m_id == id)
      {
        m_variants.erase(m_variants.begin() + i);
        break;
      }
    }
  }

  void ParameterBlock::GetCategories(VariantCategoryArray& categories, bool sortDesc)
  {
    categories.clear();
    std::unordered_map<String, bool> isCategoryAdded;
    for (const ParameterVariant& var : m_variants)
    {
      const String& name = var.m_category.Name;
      if (isCategoryAdded.find(name) == isCategoryAdded.end())
      {
        isCategoryAdded[name] = true;
        categories.push_back(var.m_category);
      }
    }

    std::sort
    (
      categories.begin(),
      categories.end(),
      [](VariantCategory& a, VariantCategory& b) -> bool
      {
        return a.Priority > b.Priority;
      }
    );
  }

  void ParameterBlock::GetByCategory(const String& category, ParameterVariantRawPtrArray& variants)
  {
    for (ParameterVariant& var : m_variants)
    {
      if (var.m_category.Name == category)
      {
        variants.push_back(&var);
      }
    }
  }

}

