/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "Node.h"

#include "BVH.h"
#include "MathUtil.h"
#include "Scene.h"
#include "ToolKit.h"
#include "Util.h"

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
    for (int i = (int) m_children.size() - 1; i >= 0; i--)
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
    Mat4 ts = glm::translate(Mat4(), val);
    TransformImp(ts, space, &m_translation, nullptr, nullptr);
  }

  void Node::Rotate(const Quaternion& val, TransformationSpace space)
  {
    if (space == TransformationSpace::TS_LOCAL)
    {
      m_orientation = m_orientation * val;
    }
    else
    {
      m_orientation = val * m_orientation;
    }

    UpdateTransformCaches();
  }

  void Node::Scale(const Vec3& val)
  {
    m_scale *= val;
    UpdateTransformCaches();
  }

  void Node::Transform(const Mat4& val, TransformationSpace space)
  {
    TransformImp(val, space, &m_translation, &m_orientation, &m_scale);
  }

  void Node::SetTransform(const Mat4& val, TransformationSpace space)
  {
    SetTransformImp(val, space, &m_translation, &m_orientation, &m_scale);
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
    if (space == TransformationSpace::TS_LOCAL)
    {
      m_orientation = val;
    }
    else
    {
      if (m_parent)
      {
        Quaternion parentWorldOrientation = m_parent->GetWorldOrientationCache();
        m_orientation                     = glm::inverse(parentWorldOrientation) * val;
      }
      else
      {
        m_orientation = val;
      }
    }

    UpdateTransformCaches();
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
    UpdateTransformCaches();
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
    Vec3 scale;
    if (preserveTransform)
    {
      ts = child->GetTransform(TransformationSpace::TS_WORLD);
    }
    else if (child->m_inheritScale)
    {
      scale = child->GetScale();
    }

    m_children.insert(m_children.begin() + index, child);
    child->m_parent = this;
    child->m_dirty  = true;
    child->SetChildrenDirty();

    if (preserveTransform)
    {
      child->SetTransform(ts, TransformationSpace::TS_WORLD);
    }
    else if (child->m_inheritScale)
    {
      child->SetScale(scale);
    }

    child->UpdateTransformCaches();
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

    child->UpdateTransformCaches();
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
    Node* node                    = new Node();

    node->m_inheritScale          = m_inheritScale;
    node->m_translation           = m_translation;
    node->m_orientation           = m_orientation;
    node->m_scale                 = m_scale;
    node->m_localCache            = m_localCache;
    node->m_worldCache            = m_worldCache;
    node->m_worldTranslationCache = m_worldTranslationCache;
    node->m_worldOrientationCache = m_worldOrientationCache;

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

  void Node::SetLocalTransforms(Vec3 translation, Quaternion rotation, Vec3 scale)
  {
    m_translation = translation;
    m_orientation = rotation;
    m_scale       = scale;

    UpdateTransformCaches();
  }

  bool Node::RequresCullFlip()
  {
    Mat3 basis = GetWorldCache();
    float det  = glm::determinant(basis);
    return det < 0.0f; // Negative determinant indicates handedness change which requires cull flip.
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

    UpdateTransformCaches();

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
      ts = val * m_localCache;
      break;
    case TransformationSpace::TS_LOCAL:
      ts = m_localCache * val;
      break;
    };

    // Extracted translation, orientation and scale is in local space,
    // no matter which transform space the new transform applied. It will always yield the local values
    // until results get multiplied with parent.
    DecomposeMatrix(ts, translation, orientation, scale);
    UpdateTransformCaches();
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
    UpdateTransformCaches();
  }

  void Node::GetTransformImp(TransformationSpace space,
                             Mat4* transform,
                             Vec3* translation,
                             Quaternion* orientation,
                             Vec3* scale)
  {
    if (m_dirty)
    {
      // This will recursively climb up in the hierarchy until it finds a clear node or clears all the tree.
      UpdateTransformCaches();
    }

    switch (space)
    {
    case TransformationSpace::TS_WORLD:
      if (m_parent != nullptr)
      {
        if (transform != nullptr)
        {
          *transform = m_worldCache;
        }
        if (translation != nullptr)
        {
          *translation = m_worldTranslationCache;
        }
        if (orientation != nullptr)
        {
          *orientation = m_worldOrientationCache;
        }
        break;
      } // Fall trough
    case TransformationSpace::TS_LOCAL:
    default:
      if (transform != nullptr)
      {
        *transform = m_localCache;
      }
      if (translation != nullptr)
      {
        *translation = m_translation;
      }
      if (orientation != nullptr)
      {
        *orientation = m_orientation;
      }
      break;
    }

    if (scale != nullptr)
    {
      *scale = m_scale;
    }
  }

  void Node::UpdateTransformCaches()
  {
    // Update local transform cache.
    Mat4 ts, rt, scl;
    scl          = glm::scale(scl, m_scale);
    rt           = glm::toMat4(m_orientation);
    ts           = glm::translate(ts, m_translation);
    m_localCache = ts * rt * scl;

    // Let all children know they need to update their parent caches.
    SetChildrenDirty();

    // Update world cache. Iteratively goes up until the root or a clean parent.
    m_worldCache = GetParentTransform() * m_localCache;

    // Update individual transform caches.
    DecomposeMatrix(m_worldCache, &m_worldTranslationCache, &m_worldOrientationCache, nullptr);

    // Invalidate entity's spatial caches.
    InvalitadeSpatialCaches();
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

      // This will climb up the hierarchy until a clean cache is found and start updating the tree downwards.
      ps = m_parent->GetTransform(TransformationSpace::TS_WORLD);

      if (!m_inheritScale)
      {
        for (int i = 0; i < 3; i++)
        {
          Vec3 v = ps[i];
          ps[i]  = Vec4(glm::normalize(v), ps[i].w);
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

  void Node::InvalitadeSpatialCaches()
  {
    if (EntityPtr ntt = m_entity.lock())
    {
      ntt->InvalidateSpatialCaches();

      for (Node* node : m_children)
      {
        node->InvalitadeSpatialCaches();
      }
    }
  }

  Quaternion Node::GetWorldOrientationCache()
  {
    if (m_dirty)
    {
      UpdateTransformCaches();
    }

    return m_worldOrientationCache;
  }

  Mat4 Node::GetWorldCache()
  {
    if (m_dirty)
    {
      UpdateTransformCaches();
    }

    return m_worldCache;
  }

} // namespace ToolKit
