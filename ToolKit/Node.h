#pragma once

#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{

  enum class TransformationSpace
  {
    TS_WORLD,
    TS_LOCAL
  };

  class TK_API Node : public Serializable
  {
    friend class Animation;
    friend class Skeleton;

   public:
    Node();
    ~Node();

    void Translate(const Vec3& val,
                   TransformationSpace space = TransformationSpace::TS_WORLD);
    void Rotate(const Quaternion& val,
                TransformationSpace space = TransformationSpace::TS_WORLD);
    void Scale(const Vec3& val);
    void Transform(const Mat4& val,
                   TransformationSpace space = TransformationSpace::TS_WORLD,
                   bool noScale              = true);
    void SetTransform(const Mat4& val,
                      TransformationSpace space = TransformationSpace::TS_WORLD,
                      bool noScale              = true);
    Mat4 GetTransform(
        TransformationSpace space = TransformationSpace::TS_WORLD);
    void SetTranslation(
        const Vec3& val,
        TransformationSpace space = TransformationSpace::TS_WORLD);
    Vec3 GetTranslation(
        TransformationSpace space = TransformationSpace::TS_WORLD);
    void SetOrientation(
        const Quaternion& val,
        TransformationSpace space = TransformationSpace::TS_WORLD);
    Quaternion GetOrientation(
        TransformationSpace space = TransformationSpace::TS_WORLD);
    void SetScale(const Vec3& val);
    Vec3 GetScale();

    /**
     * @return Normalized local transform axes of the node.
     */
    Mat3 GetTransformAxes();

    void AddChild(Node* child, bool preserveTransform = false);
    void Orphan(Node* child, bool preserveTransform = false);

    void OrphanAllChilds(bool preserveTransform);
    void OrphanChild(int index, bool preserveTransform);
    void OrphanSelf(bool preserveWorldTransform = false);
    Node* GetRoot() const;
    Node* Copy() const;
    void Serialize(XmlDocument* doc, XmlNode* parent) const;
    void DeSerialize(XmlDocument* doc, XmlNode* parent);
    void SetInheritScaleDeep(bool val);

   private:
    void TransformImp(const Mat4& val,
                      TransformationSpace space,
                      Vec3* translation,
                      Quaternion* orientation,
                      Vec3* scale);
    void SetTransformImp(const Mat4& val,
                         TransformationSpace space,
                         Vec3* translation,
                         Quaternion* orientation,
                         Vec3* scale);
    void GetTransformImp(TransformationSpace space,
                         Mat4* transform,
                         Vec3* translation,
                         Quaternion* orientation,
                         Vec3* scale);

    Mat4 GetLocalTransform() const;
    Mat4 GetParentTransform();
    void SetChildrenDirty();

   public:
    ULongID m_id;
    Node* m_parent      = nullptr;
    Entity* m_entity    = nullptr;
    bool m_inheritScale = false;
    NodePtrArray m_children;

   private:
    Vec3 m_translation;
    Quaternion m_orientation;
    Vec3 m_scale = Vec3(1.0f);
    Mat4 m_parentCache;
    bool m_dirty = true; //!< Hint for child to update its parent cache.
  };

} // namespace ToolKit
