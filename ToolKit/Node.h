#pragma once

/**
 * @file Node.h Header for Node and related structures.
 */

#include "Serialize.h"
#include "Types.h"

namespace ToolKit
{
  /**
   * Transformation space.
   */
  enum class TransformationSpace
  {
    TS_WORLD,
    TS_LOCAL
  };

  /**
   * Hierarchical transformation node class that suppose to represent 3d
   * transform of Entity in the scene.
   */
  class TK_API Node : public Serializable
  {
    friend class Animation;
    friend class Skeleton;

   public:
    Node();
    ~Node();

    /**
     * Apply translation to the node in given space.
     * @param val the delta vector for translation.
     * @param space the space to apply transform in.
     */
    void Translate(const Vec3& val,
                   TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Apply rotation to the node in given space.
     * @param val the quaternion for rotation.
     * @param space the space to apply transform in.
     */
    void Rotate(const Quaternion& val,
                TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Apply scale to the node in local space.
     * @param val delta vector for scale.
     */
    void Scale(const Vec3& val);

    /**
     * Apply given transform matrix in given space to the node.
     * @param val delta transform matrix.
     * @param space the space to apply transform in.
     * @param noScale exclude the scale value from matrix.
     */
    void Transform(const Mat4& val,
                   TransformationSpace space = TransformationSpace::TS_WORLD,
                   bool noScale              = true);

    /**
     * Sets given transform matrix in given space to the node.
     * @param val transform matrix to set.
     * @param space the space to set transform in.
     * @param noScale exclude the scale value from matrix.
     */
    void SetTransform(const Mat4& val,
                      TransformationSpace space = TransformationSpace::TS_WORLD,
                      bool noScale              = true);

    /**
     * Retrieves the transform matrix in given space.
     * @param space the retrieve transformation in.
     * @returns Calculated transformation matrix.
     */
    Mat4 GetTransform(
        TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Sets the translation value in given space.
     * @param val New translation value to set.
     * @param space The space to set the translation value in.
     */
    void SetTranslation(
        const Vec3& val,
        TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Retrieves the translation value in given space.
     * @param space The space to retrieve the translation value in.
     */
    Vec3 GetTranslation(
        TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Sets the orientation value in given space.
     * @param val New orientation value to set.
     * @param space The space to set the orientation value in.
     */
    void SetOrientation(
        const Quaternion& val,
        TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Retrieves the orientation value in given space.
     * @param space The space to retrieve the orientation value in.
     */
    Quaternion GetOrientation(
        TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Sets the scale value in local space.
     * @param val is the value for scale.
     */
    void SetScale(const Vec3& val);

    /**
     * Returns scale value.
     * @return the local space scale value.
     */
    Vec3 GetScale();

    /**
     * Calculates transform axes in given space.
     * @return Normalized local transform axes of the node.
     */
    Mat3 GetTransformAxes();

    /**
     * Adds a node as a child to the node.
     * @param child is the node to adopt.
     * @param preserveTransform is the value that indicates to keep the current
     * world space position of the node to be adopt.
     */
    void AddChild(Node* child, bool preserveTransform = false);

    /**
     * Set child's parent to null.
     * @param child to be orphaned.
     * @param preserveTransform when set true, keeps world transform of
     * orphaned child.
     */
    void Orphan(Node* child, bool preserveTransform = false);

    /**
     * Set children's parent to null.
     * @param preserveTransform when set true, keeps world transform of
     * orphaned child.
     */
    void OrphanAllChildren(bool preserveTransform);

    /**
     * Orphan the child at given index.
     * @param index of the child to be orphaned.
     * @param preserveTransform when set true, keeps world transform of
     * orphaned child.
     */
    void OrphanChild(size_t index, bool preserveTransform);

    /**
     * Unparent self from parent.
     * @param preserveWorldTransform
     */
    void OrphanSelf(bool preserveTransform = false);

    /**
     * Finds the root node.
     * @return the furthest parent node which doesn't have a parent.
     */
    Node* GetRoot() const;

    /**
     * Creates a copy of the node.
     * @return Copy of the current node.
     */
    Node* Copy() const;

    /**
     * Recursively sets inherit scale value down to all leafs.
     * @param value to be set for inherit scale.
     */
    void SetInheritScaleDeep(bool val);

    void Serialize(XmlDocument* doc, XmlNode* parent) const;
    void DeSerialize(XmlDocument* doc, XmlNode* parent);

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
    Vec3 m_translation;        //!< Local translation value.
    Quaternion m_orientation;  //!< Local orientation value.
    Vec3 m_scale = Vec3(1.0f); //!< Local scale value.
    Mat4 m_parentCache;  //!< Cached transformation of the parent hierarchy.
    bool m_dirty = true; //!< Hint for child to update its parent cache.
  };

} // namespace ToolKit
