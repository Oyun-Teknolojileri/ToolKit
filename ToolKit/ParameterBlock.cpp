#include "ParameterBlock.h"

#include <memory>
#include <utility>

#include "Util.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "DebugNew.h"
#include "Animation.h"

namespace ToolKit
{

  ParameterVariantBase::ParameterVariantBase()
  {
    m_id = GetHandleManager()->GetNextHandle();
  }

  ParameterVariantBase::~ParameterVariantBase()
  {
  }

  ParameterVariant::ParameterVariant()
  {
    *this = 0;
  }

  ParameterVariant::~ParameterVariant()
  {
  }

  ParameterVariant::ParameterVariant(const ParameterVariant& other)
  {
    *this = other;
  }

  ParameterVariant& ParameterVariant::operator=(const ParameterVariant& other)
  {
    if (m_id != other.m_id)
    {
      m_category = other.m_category;
      m_editable = other.m_editable;
      m_exposed = other.m_exposed;
      m_name = other.m_name;
      m_type = other.m_type;
      m_var = other.m_var;
      m_hint = other.m_hint;

      // Events m_onValueChangedFn intentionally not copied.
    }

    return *this;
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

  ParameterVariant::ParameterVariant(const Vec2& var)
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

  ParameterVariant::ParameterVariant(ULongID& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const MeshPtr& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const MaterialPtr& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const HdriPtr& var)
  {
    *this = var;
  }

  ParameterVariant::ParameterVariant(const AnimRecordPtrMap& var)
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
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (byte var)
  {
    m_type = VariantType::byte;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (ubyte var)
  {
    m_type = VariantType::ubyte;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (float var)
  {
    m_type = VariantType::Float;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (int var)
  {
    m_type = VariantType::Int;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (uint var)
  {
    m_type = VariantType::UInt;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator=(const Vec2& var)
  {
    m_type = VariantType::Vec2;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Vec3& var)
  {
    m_type = VariantType::Vec3;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Vec4& var)
  {
    m_type = VariantType::Vec4;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Mat3& var)
  {
    m_type = VariantType::Mat3;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const Mat4& var)
  {
    m_type = VariantType::Mat4;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const String& var)
  {
    m_type = VariantType::String;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (const char* var)
  {
    m_type = VariantType::String;
    String str = String(var);
    AsignVal(str);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator= (ULongID var)
  {
    m_type = VariantType::ULongID;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator=(const MeshPtr& var)
  {
    m_type = VariantType::MeshPtr;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator=(const MaterialPtr& var)
  {
    m_type = VariantType::MaterialPtr;
    AsignVal(var);
    return *this;
  }

  ParameterVariant& ParameterVariant::operator=(const HdriPtr& var)
  {
    m_type = VariantType::HdriPtr;
    AsignVal(var);
    return *this;
  }

  /**
  * Assign a AnimationPtr to the value of the variant.
  */
  ParameterVariant& ParameterVariant::operator=
  (
    const AnimRecordPtrMap& var
  )
  {
    m_type = VariantType::AnimRecordPtrMap;
    AsignVal(var);
    return *this;
  }

  void ParameterVariant::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* node = doc->allocate_node
    (
      rapidxml::node_element,
      XmlParamterElement.c_str()
    );

    WriteAttr
    (
      node,
      doc,
      XmlParamterTypeAttr,
      std::to_string(static_cast<int> (m_type))
    );

    WriteAttr(node, doc, "name", m_name);
    WriteAttr(node, doc, "category", m_category.Name);
    WriteAttr(node, doc, "priority", std::to_string(m_category.Priority));
    WriteAttr(node, doc, "exposed", std::to_string(m_exposed));
    WriteAttr(node, doc, "editable", std::to_string(m_editable));
    WriteAttr(node, doc, "hint.isColor", std::to_string(m_hint.isColor));
    WriteAttr
    (
      node,
      doc,
      "hint.isRanLim",
      std::to_string(m_hint.isRangeLimited)
    );
    WriteAttr(node, doc, "hint.rangeMin", std::to_string(m_hint.rangeMin));
    WriteAttr(node, doc, "hint.rangeMax", std::to_string(m_hint.rangeMax));
    WriteAttr(node, doc, "hint.increment", std::to_string(m_hint.increment));

    // Serialize data.
    switch (m_type)
    {
      case VariantType::Bool:
      {
        WriteAttr
        (
          node,
          doc,
          XmlParamterValAttr.c_str(),
          std::to_string(GetCVar<bool>())
        );
      }
      break;
      case VariantType::byte:
      {
        WriteAttr
        (
          node,
          doc,
          XmlParamterValAttr.c_str(),
          std::to_string(GetCVar<byte>())
        );
      }
      break;
      case VariantType::ubyte:
      {
        WriteAttr
        (
          node,
          doc,
          XmlParamterValAttr.c_str(),
          std::to_string(GetCVar<ubyte>())
        );
      }
      break;
      case VariantType::Float:
      {
        WriteAttr
        (
          node,
          doc,
          XmlParamterValAttr.c_str(),
          std::to_string(GetCVar<float>())
        );
      }
      break;
      case VariantType::Int:
      {
        WriteAttr
        (
          node,
          doc,
          XmlParamterValAttr.c_str(),
          std::to_string(GetCVar<int>())
        );
      }
      break;
      case VariantType::UInt:
      {
        WriteAttr
        (
          node,
          doc,
          XmlParamterValAttr.c_str(),
          std::to_string(GetCVar<uint>())
        );
      }
      break;
      case VariantType::Vec2:
      {
        WriteVec(node, doc, GetCVar<Vec2>());
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
      WriteAttr
      (
        node,
        doc,
        XmlParamterValAttr.c_str(),
        std::to_string(GetCVar<ULongID>())
      );
      break;
      case VariantType::MeshPtr:
      {
        MeshPtr res = GetCVar<MeshPtr>();
        if (res && !res->IsDynamic())
        {
          res->Save(true);
          res->SerializeRef(doc, node);
        }
      }
      break;
      case VariantType::MaterialPtr:
      {
        MaterialPtr res = GetCVar<MaterialPtr>();
        if (res && !res->IsDynamic())
        {
          res->Save(true);
          res->SerializeRef(doc, node);
        }
      }
      break;
      case VariantType::HdriPtr:
      {
        HdriPtr res = GetCVar<HdriPtr>();
        if (res && !res->IsDynamic())
        {
          res->Save(true);
          res->SerializeRef(doc, node);
        }
      }
      break;
      case VariantType::AnimRecordPtrMap:
      {
        const AnimRecordPtrMap& list = GetCVar<AnimRecordPtrMap>();
        XmlNode* listNode = CreateXmlNode(doc, "List", node);
        WriteAttr(listNode, doc, "size", std::to_string(list.size()));
        uint recordIndx = 0;
        for (auto iter = list.begin(); iter != list.end(); ++iter, recordIndx++)
        {
          const AnimRecordPtr& state = iter->second;
          XmlNode* elementNode =
            CreateXmlNode(doc, std::to_string(recordIndx), listNode);
          if (iter->first.length())
          {
            WriteAttr(elementNode, doc, "SignalName", iter->first);
          }
          if (state->m_animation)
          {
            state->m_animation->SerializeRef(doc, elementNode);
          }
        }
      }
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
    ReadAttr(parent, "exposed", m_exposed);
    ReadAttr(parent, "editable", m_editable);
    ReadAttr(parent, "hint.isColor", m_hint.isColor);
    ReadAttr(parent, "hint.isRanLim", m_hint.isRangeLimited);
    ReadAttr(parent, "hint.rangeMin", m_hint.rangeMin);
    ReadAttr(parent, "hint.rangeMax", m_hint.rangeMax);
    ReadAttr(parent, "hint.increment", m_hint.increment);

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
      case VariantType::Vec2:
      {
        Vec2 var;
        ReadVec(parent, var);
        m_var = var;
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
      case VariantType::MeshPtr:
      {
        String file = Resource::DeserializeRef(parent);
        if (file.empty())
        {
          m_var = std::make_shared<Mesh>();
        }
        else
        {
          file = MeshPath(file);
          String ext;
          DecomposePath(file, nullptr, nullptr, &ext);
          if (ext == SKINMESH)
          {
            m_var = GetMeshManager()->Create<SkinMesh>(file);
          }
          else
          {
            m_var = GetMeshManager()->Create<Mesh>(file);
          }
        }
      }
      break;
      case VariantType::MaterialPtr:
      {
        String file = Resource::DeserializeRef(parent);
        if (file.empty())
        {
          m_var = std::make_shared<Material>();
        }
        else
        {
          file = MaterialPath(file);
          m_var = GetMaterialManager()->Create<Material>(file);
        }
      }
      break;
      case VariantType::HdriPtr:
      {
        String file = Resource::DeserializeRef(parent);
        if (file.empty())
        {
          m_var = std::make_shared<Hdri>();
        }
        else
        {
          file = TexturePath(file);
          m_var = GetTextureManager()->Create<Hdri>(file);
        }
      }
      break;
      case VariantType::AnimRecordPtrMap:
      {
        XmlNode* listNode = parent->first_node("List");
        uint listSize = 0;
        ReadAttr(listNode, "size", listSize);
        AnimRecordPtrMap list;
        for (uint stateIndx = 0; stateIndx < listSize; stateIndx++)
        {
          AnimRecordPtr record = std::make_shared<AnimRecord>();
          XmlNode* elementNode =
            listNode->first_node(std::to_string(stateIndx).c_str());

          String signalName;
          ReadAttr(elementNode, "SignalName", signalName);
          String file = Resource::DeserializeRef(elementNode);
          if (!file.empty())
          {
            file = AnimationPath(file);
            record->m_animation =
              GetAnimationManager()->Create<Animation>(file);
          }
          list.insert(std::make_pair(signalName, record));
        }
        m_var = list;
      }
      break;
      default:
      assert(false && "Invalid type.");
      break;
    }
  }

  void ParameterBlock::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* blockNode = doc->allocate_node
    (
      rapidxml::node_element,
      XmlParamBlockElement.c_str()
    );

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

  void ParameterBlock::GetCategories
  (
    VariantCategoryArray& categories,
    bool sortDesc,
    bool filterByExpose
  )
  {
    categories.clear();

    std::unordered_map<String, bool> containsExposedVar;
    std::unordered_map<String, bool> isCategoryAdded;
    for (const ParameterVariant& var : m_variants)
    {
      const String& name = var.m_category.Name;
      if (var.m_exposed == true)
      {
        containsExposedVar[name] = true;
      }

      if (isCategoryAdded.find(name) == isCategoryAdded.end())
      {
        isCategoryAdded[name] = true;
        categories.push_back(var.m_category);
      }
    }

    if (filterByExpose)
    {
      categories.erase
      (
        std::remove_if
        (
          categories.begin(),
          categories.end(),
          [&containsExposedVar](const VariantCategory& vc) -> bool
          {
            // remove if not contains exposed var.
            return containsExposedVar.find(vc.Name) ==
              containsExposedVar.end();
          }
        ),
        categories.end()
      );
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

  void ParameterBlock::GetByCategory
  (
    const String& category,
    ParameterVariantRawPtrArray& variants
  )
  {
    for (ParameterVariant& var : m_variants)
    {
      if (var.m_category.Name == category)
      {
        variants.push_back(&var);
      }
    }
  }

}  // namespace ToolKit
