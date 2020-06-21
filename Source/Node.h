#pragma once

#include "ToolKit.h"

namespace ToolKit
{

	enum class TransformationSpace
	{
		TS_WORLD,
		TS_PARENT,
		TS_LOCAL
	};

	class Node
	{
		friend class Animation;
		friend class Skeleton;

	public:
		Node();
		~Node();

		void Translate(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		void Rotate(const Quaternion& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		void Scale(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		void Transform(const Mat4& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		void SetTransform(const Mat4& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		Mat4 GetTransform(TransformationSpace space = TransformationSpace::TS_PARENT);
		void SetTranslation(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		Vec3 GetTranslation(TransformationSpace space = TransformationSpace::TS_PARENT);
		void SetOrientation(const Quaternion& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		Quaternion GetOrientation(TransformationSpace space = TransformationSpace::TS_PARENT);
		void SetScale(const Vec3& val, TransformationSpace space = TransformationSpace::TS_PARENT);
		Vec3 GetScale(TransformationSpace space = TransformationSpace::TS_PARENT);
		void AddChild(Node* child, bool preserveTransform = false);
		void Orphan(Node* child, bool preserveTransform = false);
		void OrphanSelf(bool preserveWorldTransform = false);
		Node* GetRoot() const;
		Node* GetCopy() const;
		void Serialize(XmlDocument* doc, XmlNode* parent) const;
		void DeSerialize(XmlDocument* doc, XmlNode* parent);

	private:
		void TransformImp(const Mat4& val, TransformationSpace space, Vec3* translation, Quaternion* orientation, Vec3* scale);
		void SetTransformImp(const Mat4& val, TransformationSpace space, Vec3* translation, Quaternion* orientation, Vec3* scale);
		void GetTransformImp(TransformationSpace space, Mat4* transform, Vec3* translation, Quaternion* orientation, Vec3* scale);
		Mat4 GetLocalTransform() const;
		Mat4 GetParentTransform();
		void SetChildrenDirty();

	public:
		NodeId m_id;
		Node* m_parent = nullptr;
		Entity* m_entity = nullptr;
		bool m_inheritScale = false;
		bool m_inheritOnlyTranslate = false;
		NodePtrArray m_children;
	
	private:
		Vec3 m_translation;
		Quaternion m_orientation;
		Vec3 m_scale = Vec3(1.0f);
		static NodeId m_nextId;

		Mat4 m_parentCache;
		bool m_dirty = true; // Hint for child to update its parent cache.
	};

}