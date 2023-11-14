/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "Node.h"

#include "MathUtil.h"
#include "ToolKit.h"
#include "Util.h"

#include "DebugNew.h"

namespace ToolKit
{

  Node::Node() : m_scale(Vec3(1.0f))
  {
    m_id           = GetHandleManager()->GenerateHandle();
    m_parent       = nullptr;
    m_inheritScale = false;
    m_dirty        = true;
  }

  Node::~Node()
  {
    OrphanSelf(true);
    for (int i = static_cast<int>(m_children.size()) - 1; i >= 0; i--)
    {
      Orphan(m_children[i], true);
    }

    if (HandleManager* handleManager = GetHandleManager())
    {
      handleManager->ReleaseHandle(m_id);
    }
  }

  void Node::Translate(const Vec3& val, TransformationSpace space)
  {
    Vec3 tmpScl = m_scale;
    m_scale     = Vec3(1.0f);

    Mat4 ts     = glm::translate(Mat4(), val);
    TransformImp(ts, space, &m_translation, nullptr, nullptr);

    m_scale = tmpScl;
  }

  void Node::Rotate(const Quaternion& val, TransformationSpace space)
  {
    Vec3 tmpScl = m_scale;
    m_scale     = Vec3(1.0f);

    Mat4 ts     = glm::toMat4(val);
    TransformImp(ts, space, nullptr, &m_orientation, nullptr);

    m_scale = tmpScl;
  }

  void Node::Scale(const Vec3& val)
  {
    m_scale *= val;
    SetChildrenDirty();
  }

  void Node::Transform(const Mat4& val, TransformationSpace space, bool noScale)
  {
    Vec3 tmpScl = m_scale;
    if (noScale)
    {
      m_scale = Vec3(1.0f);
    }

    TransformImp(val, space, &m_translation, &m_orientation, noScale ? nullptr : &m_scale);

    if (noScale)
    {
      m_scale = tmpScl;
    }
  }

  void Node::SetTransform(const Mat4& val, TransformationSpace space, bool noScale)
  {
    Vec3 tmpScl = m_scale;
    if (noScale)
    {
      m_scale = Vec3(1.0f);
    }

    SetTransformImp(val, space, &m_translation, &m_orientation, noScale ? nullptr : &m_scale);

    if (noScale)
    {
      m_scale = tmpScl;
    }
  }

  Mat4 Node::GetTransform(TransformationSpace space)
  {
    Mat4 ts;
    GetTransformImp(space, &ts, nullptr, nullptr, nullptr);
    return ts;
  }

  void Node::SetTranslation(const Vec3& val, TransformationSpace space)
  {
    Mat4 ts = glm::translate(Mat4(), val);
    SetTransformImp(ts, space, &m_translation, nullptr, nullptr);
  }

  Vec3 Node::GetTranslation(TransformationSpace space)
  {
    Vec3 t;
    GetTransformImp(space, nullptr, &t, nullptr, nullptr);
    return t;
  }

  void Node::SetOrientation(const Quaternion& val, TransformationSpace space)
  {
    Mat4 ts = glm::toMat4(val);
    SetTransformImp(ts, space, nullptr, &m_orientation, nullptr);
  }

  Quaternion Node::GetOrientation(TransformationSpace space)
  {
    Quaternion q;
    GetTransformImp(space, nullptr, nullptr, &q, nullptr);
    return q;
  }

  void Node::SetScale(const Vec3& val)
  {
    m_scale = val;
    SetChildrenDirty();
  }

  Vec3 Node::GetScale() { return m_scale; }

  Mat3 Node::GetTransformAxes()
  {
    Quaternion q = GetOrientation();
    Mat3 axes    = glm::toMat3(q);

    return axes;
  }

  void Node::InsertChild(Node* child, int index, bool preserveTransform)
  {
    bool canInsert  = index <= m_children.size() && index >= 0;
    canInsert      &= child->m_id != m_id && child->m_parent == nullptr;
    assert(canInsert);

    if (!canInsert)
    {
      return;
    }
    Mat4 ts;
    if (preserveTransform)
    {
      ts = child->GetTransform(TransformationSpace::TS_WORLD);
    }

    m_children.insert(m_children.begin() + index, child);
    child->m_parent = this;
    child->m_dirty  = true;
    child->SetChildrenDirty();

    if (preserveTransform)
    {
      child->SetTransform(ts, TransformationSpace::TS_WORLD);
    }
  }

  void Node::AddChild(Node* child, bool preserveTransform)
  {
    InsertChild(child, (int) m_children.size(), preserveTransform);
  }

  void Node::OrphanChild(size_t index, bool preserveTransform)
  {
    if (index >= m_children.size())
    {
      return;
    }

    Mat4 ts;
    Node* child = m_children[index];
    if (preserveTransform)
    {
      ts = child->GetTransform(TransformationSpace::TS_WORLD);
    }

    child->m_parent = nullptr;
    child->m_dirty  = true;
    child->SetChildrenDirty();
    m_children.erase(m_children.begin() + index);

    if (preserveTransform)
    {
      child->SetTransform(ts, TransformationSpace::TS_WORLD);
    }
  }

  void Node::OrphanAllChildren(bool preserveTransform)
  {
    while (!m_children.empty())
    {
      // remove last children
      OrphanChild(m_children.size() - 1ull, preserveTransform);
    }
  }

  void Node::Orphan(Node* child, bool preserveTransform)
  {
    for (size_t i = 0; i < m_children.size(); i++)
    {
      if (m_children[i] == child)
      {
        OrphanChild(i, preserveTransform);
        return;
      }
    }
  }

  void Node::OrphanSelf(bool preserveTransform)
  {
    if (m_parent)
    {
      m_parent->Orphan(this, preserveTransform);
    }
  }

  Node* Node::GetRoot() const
  {
    if (m_parent == nullptr)
    {
      return nullptr;
    }

    return m_parent->GetRoot();
  }

  Node* Node::Copy() const
  {
    // Does not preserve parent / child relation
    // Look at Util/DeepCopy for preserving hierarchy.
    Node* node           = new Node();

    node->m_inheritScale = m_inheritScale;
    node->m_translation  = m_translation;
    node->m_orientation  = m_orientation;
    node->m_scale        = m_scale;

    return node;
  }

  EntityPtr Node::ParentEntity()
  {
    if (m_parent != nullptr)
    {
      if (EntityPtr parentNtt = m_parent->m_entity.lock())
      {
        return parentNtt;
      }
    }
    return nullptr;
  }

  XmlNode* Node::SerializeImp(XmlDocument* doc, XmlNode* parent) const
  {
    XmlNode* node = CreateXmlNode(doc, XmlNodeElement, parent);
    WriteAttr(node, doc, XmlNodeInheritScaleAttr, std::to_string((int) m_inheritScale));

    XmlNode* tNode = CreateXmlNode(doc, XmlTranslateElement, node);
    WriteVec(tNode, doc, m_translation);

    tNode = CreateXmlNode(doc, XmlRotateElement, node);
    WriteVec(tNode, doc, m_orientation);

    tNode = CreateXmlNode(doc, XmlScaleElement, node);
    WriteVec(tNode, doc, m_scale);

    return node;
  }

  XmlNode* Node::DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent)
  {
    XmlNode* node = parent;
    if (XmlAttribute* attr = node->first_attribute(XmlNodeInheritScaleAttr.c_str()))
    {
      String val     = attr->value();
      m_inheritScale = (bool) std::atoi(val.c_str());
    }

    if (XmlNode* n = node->first_node(XmlTranslateElement.c_str()))
    {
      ReadVec(n, m_translation);
    }

    if (XmlNode* n = node->first_node(XmlRotateElement.c_str()))
    {
      ReadVec(n, m_orientation);
    }

    if (XmlNode* n = node->first_node(XmlScaleElement.c_str()))
    {
      ReadVec(n, m_scale);
    }

    return nullptr;
  }

  void Node::SetInheritScaleDeep(bool val)
  {
    m_inheritScale = val;
    m_dirty        = true;
    for (Node* n : m_children)
    {
      n->SetInheritScaleDeep(val);
    }
  }

  void Node::TransformImp(const Mat4& val,
                          TransformationSpace space,
                          Vec3* translation,
                          Quaternion* orientation,
                          Vec3* scale)
  {
    Mat4 ts;
    switch (space)
    {
    case TransformationSpace::TS_WORLD:
      ts = val * GetLocalTransform();
      break;
    case TransformationSpace::TS_LOCAL:
      ts = GetLocalTransform() * val;
      break;
    }

    DecomposeMatrix(ts, translation, orientation, scale);
    SetChildrenDirty();
  }

  void Node::SetTransformImp(const Mat4& val,
                             TransformationSpace space,
                             Vec3* translation,
                             Quaternion* orientation,
                             Vec3* scale)
  {
    Mat4 ts;
    switch (space)
    {
    case TransformationSpace::TS_WORLD:
      if (m_parent != nullptr)
      {
        Mat4 ps = GetParentTransform();
        ts      = glm::inverse(ps) * val;
        break;
      } // Fall trough
    case TransformationSpace::TS_LOCAL:
      ts = val;
    }

    DecomposeMatrix(ts, translation, orientation, scale);
    SetChildrenDirty();
  }

  void Node::GetTransformImp(TransformationSpace space,
                             Mat4* transform,
                             Vec3* translation,
                             Quaternion* orientation,
                             Vec3* scale)
  {
    switch (space)
    {
    case TransformationSpace::TS_WORLD:
      if (m_parent != nullptr)
      {
        Mat4 ts = GetLocalTransform();
        Mat4 ps = GetParentTransform();
        ts      = ps * ts;
        if (transform != nullptr)
        {
          *transform = ts;
        }
        DecomposeMatrix(ts, translation, orientation, scale);
        break;
      } // Fall trough
    case TransformationSpace::TS_LOCAL:
    default:
      if (transform != nullptr)
      {
        *transform = GetLocalTransform();
      }
      if (translation != nullptr)
      {
        *translation = m_translation;
      }
      if (orientation != nullptr)
      {
        *orientation = m_orientation;
      }
      if (scale != nullptr)
      {
        *scale = m_scale;
      }
      break;
    }
  }

  Mat4 Node::GetLocalTransform() const
  {
    Mat4 ts, rt, scl;
    scl = glm::scale(scl, m_scale);
    rt  = glm::toMat4(m_orientation);
    ts  = glm::translate(ts, m_translation);
    return ts * rt * scl;
  }

  Mat4 Node::GetParentTransform()
  {
    Mat4 ps;
    if (m_parent != nullptr)
    {
      if (!m_dirty)
      {
        return m_parentCache;
      }

      ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);

      if (!m_inheritScale)
      {
        for (int i = 0; i < 3; i++)
        {
          Vec3 v    = ps[i];
          ps[i].xyz = glm::normalize(v);
        }
      }

      m_parentCache = ps;
    }

    m_dirty = false;
    return ps;
  }

  void Node::SetChildrenDirty()
  {
    for (Node* c : m_children)
    {
      c->m_dirty = true;
      c->SetChildrenDirty();
    }
  }

} // namespace ToolKit
