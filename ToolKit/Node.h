/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

/**
 * @file Node.h Header for Node and related structures.
 */

#include "Serialize.h"

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
   public:
    Node();
    ~Node();

    /**
     * Apply translation to the node in given space.
     * @param val the delta vector for translation.
     * @param space the space to apply transform in.
     */
    void Translate(const Vec3& val, TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Apply rotation to the node in given space.
     * @param val the quaternion for rotation.
     * @param space the space to apply transform in.
     */
    void Rotate(const Quaternion& val, TransformationSpace space = TransformationSpace::TS_WORLD);

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
    void Transform(const Mat4& val, TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Sets given transform matrix in given space to the node.
     * @param val transform matrix to set.
     * @param space the space to set transform in.
     * @param noScale exclude the scale value from matrix.
     */
    void SetTransform(const Mat4& val, TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Retrieves the transform matrix in given space.
     * @param space the retrieve transformation in.
     * @returns Calculated transformation matrix.
     */
    Mat4 GetTransform(TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Sets the translation value in given space.
     * @param val New translation value to set.
     * @param space The space to set the translation value in.
     */
    void SetTranslation(const Vec3& val, TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Retrieves the translation value in given space.
     * @param space The space to retrieve the translation value in.
     */
    Vec3 GetTranslation(TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Sets the orientation value in given space.
     * @param val New orientation value to set.
     * @param space The space to set the orientation value in.
     */
    void SetOrientation(const Quaternion& val, TransformationSpace space = TransformationSpace::TS_WORLD);

    /**
     * Retrieves the orientation value in given space.
     * @param space The space to retrieve the orientation value in.
     */
    Quaternion GetOrientation(TransformationSpace space = TransformationSpace::TS_WORLD);

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
     * Inserts a childs in given index.
     * @param child child that you want to insert.
     * @param index index that you want to insert.
     * @param preserveTransform is the value that indicates to keep the current
     * world space position of the node to be adopt.
     */
    void InsertChild(Node* child, int index, bool preserveTransform = false);

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

    /**
     * Returns parent entity if any exist.
     * @return Parent Entity.
     */
    EntityPtr ParentEntity();

    /**
     * Getter function for owner entity.
     * @return Owner EntityPtr.
     */
    EntityPtr OwnerEntity() const { return m_entity.lock(); }

    /**
     * Setter function for owner entity.
     * @param owner owning EntityPtr.
     */
    void OwnerEntity(EntityPtr owner) { m_entity = owner; }

    /**
     * Sets the local transforms of the node.
     */
    void SetLocalTransforms(Vec3 translation, Quaternion rotation, Vec3 scale);

    /** Odd number of negative values in scale requires back / front culling to be flipped for proper winding order. */
    bool RequresCullFlip();

    XmlNode* SerializeImp(XmlDocument* doc, XmlNode* parent) const;
    XmlNode* DeSerializeImp(const SerializationFileInfo& info, XmlNode* parent);

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

    void UpdateTransformCaches();
    Mat4 GetParentTransform();
    void SetChildrenDirty();
    void InvalitadeSpatialCaches();
    Quaternion GetWorldOrientationCache();
    Mat4 GetWorldCache();

   public:
    ULongID m_id;
    Node* m_parent;
    NodeRawPtrArray m_children;
    bool m_inheritScale;

   private:
    EntityWeakPtr m_entity;   //!< Entity that owns this node.
    Vec3 m_translation;       //!< Local translation value.
    Quaternion m_orientation; //!< Local orientation value.
    Vec3 m_scale;             //!< Local scale value.

    /** Local transform matrix cache.*/
    Mat4 m_localCache;
    /** Cached transformation of the parent hierarchy. Never access directly. It may be dirty. */
    Mat4 m_parentCache;
    /** World transform matrix cache. Never access directly. It may be dirty. */
    Mat4 m_worldCache;
    /** World translation cache. Never access directly. It may be dirty. */
    Vec3 m_worldTranslationCache;
    /** World orientation cache. Never access directly. It may be dirty. */
    Quaternion m_worldOrientationCache;

    bool m_dirty; //!< Hint for child to update its parent cache.
  };

} // namespace ToolKit
