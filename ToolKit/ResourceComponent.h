#pragma once

#include "Component.h"
#include "MathUtil.h"
#include "Types.h"

#include <memory>
#include <vector>

namespace ToolKit
{

  typedef std::shared_ptr<class MeshComponent> MeshComponentPtr;
  typedef std::vector<MeshComponentPtr> MeshComponentPtrArray;
  static VariantCategory MeshComponentCategory{"Mesh Component", 90};

  class TK_API MeshComponent : public Component
  {
   public:
    /**
     * Auto generated code for type information.
     */
    TKComponentType(MeshComponent);

    /**
     * Empty constructor.
     */
    MeshComponent();

    /**
     * Empty destructor.
     */
    virtual ~MeshComponent();

    /**
     * Creates a copy of the MeshComponent. Contained Mesh does not get
     * copied but referenced. However Material is copied and will be serialized
     * to the scene if the containing Entity gets serialized.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MeshComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    /**
     * Gets the bounding box of the contained Mesh.
     * @return BoundingBox of the contained Mesh.
     */
    BoundingBox GetAABB();

    /**
     * Initiates the MeshComponent and underlying Mesh and Material resources.
     */
    void Init(bool flushClientSideArray);

   public:
    TKDeclareParam(MeshPtr, Mesh); //!< Component's Mesh resource.
    TKDeclareParam(bool, CastShadow);

   private:
    BoundingBox m_aabb = {};
  };

  typedef std::shared_ptr<class MaterialComponent> MaterialComponentPtr;
  typedef std::vector<MaterialComponentPtr> MaterialComponentPtrArray;

  static VariantCategory MaterialComponentCategory{"Material Component", 90};

  class TK_API MaterialComponent : public Component
  {
   public:
    TKComponentType(MaterialComponent);

    MaterialComponent();
    virtual ~MaterialComponent();

    /**
     * Creates a copy of the MaterialComponent. Contained Material is copied
     * and will be serialized to the scene if the containing Entity gets
     * serialized.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MaterialComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);

   public:
    /**
     * Component's material resource. In case this object is not null, Renderer
     * picks this material to render the mesh otherwise falls back to Material
     * within the Mesh.
     */
    TKDeclareParam(MaterialPtr, Material);
  };

  typedef std::shared_ptr<class EnvironmentComponent> EnvironmentComponentPtr;
  static VariantCategory EnvironmentComponentCategory{"Environment Component",
                                                      90};

  class TK_API EnvironmentComponent : public Component
  {
   public:
    TKComponentType(EnvironmentComponent);

    EnvironmentComponent();
    virtual ~EnvironmentComponent();

    ComponentPtr Copy(Entity* ntt) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    BoundingBox GetBBox();

    void Init(bool flushClientSideArray);

   private:
    void ParameterConstructor();
    void ParameterEventConstructor();

   public:
    TKDeclareParam(HdriPtr, Hdri);
    TKDeclareParam(Vec3, Size);
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(bool, Illuminate);
    TKDeclareParam(float, Intensity);
    TKDeclareParam(float, Exposure);
  };

  static VariantCategory AnimRecordComponentCategory{
      "Animation Record Component", 90};
  typedef std::shared_ptr<class AnimControllerComponent>
      AnimControllerComponentPtr;

  /**
   * The component that stores animation records for the entity
   */
  class TK_API AnimControllerComponent : public Component
  {
   public:
    TKComponentType(AnimControllerComponent);

    /**
     * Empty constructor.
     */
    AnimControllerComponent();
    virtual ~AnimControllerComponent();

    ComponentPtr Copy(Entity* ntt) override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

    void Play(const String& recordName);
    void Stop();
    void Pause();
    AnimRecordPtr GetActiveRecord();
    AnimRecordPtr GetAnimRecord(const String& signalName);
    void AddSignal(const String& signalName, AnimRecordPtr record);
    void RemoveSignal(const String& signalName);

   public:
    TKDeclareParam(AnimRecordPtrMap, Records);

   private:
    AnimRecordPtr activeRecord;
  };

  static VariantCategory SkeletonComponentCategory{"Skeleton Component", 90};
  typedef std::shared_ptr<class SkeletonComponent> SkeletonComponentPtr;

  /**
   * The component that stores skeleton resource reference and dynamic bone
      transformation info
   */
  class DynamicBoneMap;
  class TK_API SkeletonComponent : public Component
  {
   public:
    TKComponentType(SkeletonComponent);

    /**
     * Empty constructor.
     */
    SkeletonComponent();
    virtual ~SkeletonComponent();
    void Init();

    ComponentPtr Copy(Entity* ntt) override;
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;

   public:
    TKDeclareParam(SkeletonPtr, SkeletonResource);
    DynamicBoneMap* m_map;
    bool isDirty = true;
  };

  typedef std::shared_ptr<class MultiMaterialComponent> MultiMaterialPtr;
  typedef std::vector<MultiMaterialPtr> MultiMaterialPtrArray;

  static VariantCategory MultiMaterialCompCategory{"Multi-Material Component",
                                                   90};

  class TK_API MultiMaterialComponent : public Component
  {
   public:
    TKComponentType(MultiMaterialComponent);

    MultiMaterialComponent();
    virtual ~MultiMaterialComponent();

    /**
     * Creates a copy of the MultiMaterialComponent.
     * @param ntt Parent Entity of the component.
     * @return Copy of the MultiMaterialComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    void DeSerialize(XmlDocument* doc, XmlNode* parent) override;
    void Serialize(XmlDocument* doc, XmlNode* parent) const override;
    void AddMaterial(MaterialPtr mat);
    void RemoveMaterial(uint index);
    const MaterialPtrArray& GetMaterialList() const;
    MaterialPtrArray& GetMaterialList();
    void UpdateMaterialList(MeshComponentPtr meshComp);

   private:
    MaterialPtrArray materials;
  };

  typedef std::shared_ptr<class AABBOverrideComponent> AABBOverrideComponentPtr;
  typedef std::vector<AABBOverrideComponentPtr> AABBOverrideComponentPtrArray;

  static VariantCategory AABBOverrideCompCategory{"AABB Override Component",
                                                  90};

  class TK_API AABBOverrideComponent : public Component
  {
   public:
    TKComponentType(AABBOverrideComponent);

    AABBOverrideComponent();
    virtual ~AABBOverrideComponent();

    /**
     * Creates a copy of the AABB Override Component.
     * @param ntt Parent Entity of the component.
     * @return Copy of the AABBOverrideComponent.
     */
    ComponentPtr Copy(Entity* ntt) override;

    void Init(bool flushClientSideArray);
    BoundingBox GetAABB();
    // AABB should be in entity space (not world space)
    void SetAABB(BoundingBox aabb);

   private:
    TKDeclareParam(Vec3, PositionOffset);
    TKDeclareParam(Vec3, Size);
  };
} //  namespace ToolKit
