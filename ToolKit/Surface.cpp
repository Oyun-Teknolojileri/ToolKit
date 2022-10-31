#include "Surface.h"

#include "Material.h"
#include "Mesh.h"
#include "Node.h"
#include "Texture.h"
#include "ToolKit.h"
#include "Viewport.h"

#include <memory>

#include "DebugNew.h"

namespace ToolKit
{
  Surface::Surface()
  {
    ComponentConstructor();
    ParameterConstructor();
    ParameterEventConstructor();
  }

  Surface::Surface(TexturePtr texture, const Vec2& pivotOffset) : Surface()
  {
    GetMaterialComponent()->GetMaterialVal()->m_diffuseTexture = texture;
    SetPivotOffsetVal(pivotOffset);
    SetSizeFromTexture();
  }

  Surface::Surface(TexturePtr texture, const SpriteEntry& entry) : Surface()
  {
    GetMaterialComponent()->GetMaterialVal()->m_diffuseTexture = texture;
    SetSizeFromTexture();
  }

  Surface::Surface(const String& textureFile, const Vec2& pivotOffset)
      : Surface()
  {
    GetMaterialComponent()->GetMaterialVal()->m_diffuseTexture =
        GetTextureManager()->Create<Texture>(textureFile);

    SetPivotOffsetVal(pivotOffset);
    SetSizeFromTexture();
  }

  Surface::Surface(const Vec2& size, const Vec2& offset) : Surface()
  {
    SetSizeVal(size);
    SetPivotOffsetVal(offset);
  }

  Surface::~Surface()
  {
  }

  EntityType Surface::GetType() const
  {
    return EntityType::Entity_Surface;
  }

  void Surface::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Entity::Serialize(doc, parent);
    parent        = parent->last_node();
    XmlNode* node = CreateXmlNode(doc, "Anchor", parent);

    for (int i = 0; i < 4; i++)
    {
      WriteAttr(node,
                doc,
                "ratios" + std::to_string(i),
                std::to_string(m_anchorParams.m_anchorRatios[i]));
      WriteAttr(node,
                doc,
                "offsets" + std::to_string(i),
                std::to_string(m_anchorParams.m_offsets[i]));
    }
  }

  void Surface::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    if (XmlNode* node = parent->first_node("Anchor"))
    {
      for (int i = 0; i < 4; i++)
      {
        ReadAttr(node,
                 "ratios" + std::to_string(i),
                 m_anchorParams.m_anchorRatios[i]);
        ReadAttr(
            node, "offsets" + std::to_string(i), m_anchorParams.m_offsets[i]);
      }
    }
    ParameterEventConstructor();

    // Re assign default material.
    MaterialComponentPtr matCom = GetMaterialComponent();
    if (matCom->GetMaterialVal()->IsDynamic())
    {
      matCom->SetMaterialVal(GetMaterialManager()->GetCopyOfUIMaterial());
    }

    CreateQuat();
  }

  void Surface::UpdateGeometry(bool byTexture)
  {
    if (byTexture)
    {
      SetSizeFromTexture();
    }

    MeshPtr mesh = GetMeshComponent()->GetMeshVal();
    mesh->UnInit();
    CreateQuat();
    mesh->Init();
  }

  void Surface::ComponentConstructor()
  {
    MeshComponent* meshComponent         = new MeshComponent();
    meshComponent->ParamMesh().m_exposed = false;
    meshComponent->SetCastShadowVal(false);
    AddComponent(meshComponent);

    MaterialComponent* materialComponent         = new MaterialComponent();
    materialComponent->ParamMaterial().m_exposed = false;
    AddComponent(materialComponent);

    MaterialPtr material = GetMaterialManager()->GetCopyOfUIMaterial();
    materialComponent->SetMaterialVal(material);
  }

  void Surface::ParameterConstructor()
  {
    Size_Define({150.0f, 50.0f},
                SurfaceCategory.Name,
                SurfaceCategory.Priority,
                true,
                true);

    PivotOffset_Define({0.5f, 0.5f},
                       SurfaceCategory.Name,
                       SurfaceCategory.Priority,
                       true,
                       true);

    Material_Define(GetMaterialComponent()->GetMaterialVal(),
                    SurfaceCategory.Name,
                    SurfaceCategory.Priority,
                    true,
                    true);
  }

  void Surface::ParameterEventConstructor()
  {
    ParamSize().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void {
          UpdateGeometry(false);
        });

    ParamPivotOffset().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void {
          UpdateGeometry(false);
        });

    ParamMaterial().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void {
          GetMaterialComponent()->SetMaterialVal(std::get<MaterialPtr>(newVal));
        });

    GetMeshComponent()->ParamMesh().m_exposed       = false;
    GetMeshComponent()->ParamCastShadow().m_exposed = false;
  }

  Entity* Surface::CopyTo(Entity* other) const
  {
    Entity* cpy = Entity::CopyTo(other);

    // Create an independent mesh.
    MeshPtr mesh = std::make_shared<Mesh>();
    cpy->GetMeshComponent()->SetMeshVal(mesh);

    Surface* surf = static_cast<Surface*>(cpy);
    surf->UpdateGeometry(false);

    return cpy;
  }

  void Surface::ResetCallbacks()
  {
    m_onMouseEnter = nullptr;
    m_onMouseExit  = nullptr;
    m_onMouseOver  = nullptr;
    m_onMouseClick = nullptr;
  }

  void Surface::CreateQuat()
  {
    float width  = GetSizeVal().x;
    float height = GetSizeVal().y;
    float depth  = 0;
    Vec2 absOffset =
        Vec2(GetPivotOffsetVal().x * width, GetPivotOffsetVal().y * height);

    VertexArray vertices;
    vertices.resize(6);
    vertices[0].pos = Vec3(-absOffset.x, -absOffset.y, depth);
    vertices[0].tex = Vec2(0.0f, 1.0f);
    vertices[1].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[1].tex = Vec2(1.0f, 1.0f);
    vertices[2].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[2].tex = Vec2(0.0f, 0.0f);

    vertices[3].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[3].tex = Vec2(1.0f, 1.0f);
    vertices[4].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
    vertices[4].tex = Vec2(1.0f, 0.0f);
    vertices[5].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[5].tex = Vec2(0.0f, 0.0f);

    MeshPtr mesh               = GetMeshComponent()->GetMeshVal();
    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
  }

  void Surface::CreateQuat(const SpriteEntry& val)
  {
    assert(false && "Old function needs to be re factored.");

    MeshPtr mesh = GetMeshComponent()->GetMeshVal();
    float imageWidth =
        static_cast<float>(mesh->m_material->m_diffuseTexture->m_width);

    float imageHeight =
        static_cast<float>(mesh->m_material->m_diffuseTexture->m_height);

    Rect<float> textureRect;
    textureRect.x =
        static_cast<float>(val.rectangle.x) / static_cast<float>(imageWidth);

    textureRect.height = (static_cast<float>(val.rectangle.height) /
                          static_cast<float>(imageHeight));

    textureRect.y = 1.0f -
                    (static_cast<float>(val.rectangle.y) /
                     static_cast<float>(imageHeight)) -
                    textureRect.height;

    textureRect.width = static_cast<float>(val.rectangle.width) /
                        static_cast<float>(imageWidth);

    float depth    = 0.0f;
    float width    = static_cast<float>(val.rectangle.width);
    float height   = static_cast<float>(val.rectangle.height);
    Vec2 absOffset = Vec2(val.offset.x * val.rectangle.width,
                          val.offset.y * val.rectangle.height);

    VertexArray vertices;
    vertices.resize(6);
    vertices[0].pos = Vec3(-absOffset.x, -absOffset.y, depth);
    vertices[0].tex = Vec2(textureRect.x, 1.0f - textureRect.y);
    vertices[1].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[1].tex =
        Vec2(textureRect.x + textureRect.width, 1.0f - textureRect.y);

    vertices[2].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[2].tex =
        Vec2(textureRect.x, 1.0f - (textureRect.y + textureRect.height));

    vertices[3].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[3].tex =
        Vec2(textureRect.x + textureRect.width, 1.0f - textureRect.y);

    vertices[4].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
    vertices[4].tex = Vec2(textureRect.x + textureRect.width,
                           1.0f - (textureRect.y + textureRect.height));

    vertices[5].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[5].tex =
        Vec2(textureRect.x, 1.0f - (textureRect.y + textureRect.height));

    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
  }

  void Surface::SetSizeFromTexture()
  {
    if (TexturePtr dt = GetMaterialVal()->m_diffuseTexture)
    {
      SetSizeVal(Vec2(dt->m_width, dt->m_height));
    }
  }

  void Surface::CalculateAnchorOffsets(Vec3 canvas[4], Vec3 surface[4])
  {
    if (canvas == nullptr || surface == nullptr ||
        m_node->m_parent == nullptr || m_node->m_parent->m_entity == nullptr ||
        m_node->m_parent->m_entity->GetType() != EntityType::Entity_Canvas)
      return;

    Canvas* canvasPanel = static_cast<Canvas*>(m_node->m_parent->m_entity);

    const BoundingBox bb = canvasPanel->GetAABB(true);
    const float w        = bb.GetWidth();
    const float h        = bb.GetHeight();

    Vec3 pos =
        canvasPanel->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    pos.x = bb.min.x;
    pos.y = bb.max.y;

    const Vec3 axis[3] = {
        {1.f, 0.f, 0.f},
        {0.f, 1.f, 0.f},
        {0.f, 0.f, 1.f}
    };

    canvas[0] = pos;
    canvas[0] -= axis[1] * ((m_anchorParams.m_anchorRatios[2]) * h);
    canvas[0] += axis[0] * ((m_anchorParams.m_anchorRatios[0]) * w);
    canvas[0].z = 0.f;

    canvas[1] = pos;
    canvas[1] -= axis[1] * ((m_anchorParams.m_anchorRatios[2]) * h);
    canvas[1] += axis[0] * ((1.f - m_anchorParams.m_anchorRatios[1]) * w);
    canvas[1].z = 0.f;

    canvas[2] = pos;
    canvas[2] -= axis[1] * ((1.f - m_anchorParams.m_anchorRatios[3]) * h);
    canvas[2] += axis[0] * ((m_anchorParams.m_anchorRatios[0]) * w);
    canvas[2].z = 0.f;

    canvas[3] = pos;
    canvas[3] -= axis[1] * ((1.f - m_anchorParams.m_anchorRatios[3]) * h);
    canvas[3] += axis[0] * ((1.f - m_anchorParams.m_anchorRatios[1]) * w);
    canvas[3].z = 0.f;

    const BoundingBox surfaceBB = GetAABB(true);
    surface[0]                  = Vec3(surfaceBB.min.x, surfaceBB.max.y, 0.f);
    surface[1]                  = Vec3(surfaceBB.max.x, surfaceBB.max.y, 0.f);
    surface[2]                  = Vec3(surfaceBB.min.x, surfaceBB.min.y, 0.f);
    surface[3]                  = Vec3(surfaceBB.max.x, surfaceBB.min.y, 0.f);
  }

  // Button
  //////////////////////////////////////////

  Button::Button()
  {
    ParameterConstructor();
    ParameterEventConstructor();
    ResetCallbacks();
  }

  Button::Button(const Vec2& size) : Surface(size)
  {
    ParameterConstructor();
    ParameterEventConstructor();
    ResetCallbacks();
  }

  Button::Button(const TexturePtr& buttonImage, const TexturePtr& hoverImage)
      : Surface(buttonImage, Vec2())
  {
    ParameterConstructor();
    ParameterEventConstructor();
    GetButtonMaterialVal()->m_diffuseTexture = buttonImage;
    GetHoverMaterialVal()->m_diffuseTexture  = hoverImage;
    ResetCallbacks();
  }

  Button::~Button()
  {
  }

  EntityType Button::GetType() const
  {
    return EntityType::Entity_Button;
  }

  void Button::Serialize(XmlDocument* doc, XmlNode* parent) const
  {
    Surface::Serialize(doc, parent);
  }

  void Button::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Surface::DeSerialize(doc, parent);
    ParameterEventConstructor();
  }

  void Button::ResetCallbacks()
  {
    Surface::ResetCallbacks();

    m_onMouseEnterLocal = [this](Event* e, Entity* ntt) -> void {
      SetMaterialVal(GetHoverMaterialVal());
    };
    m_onMouseEnter = m_onMouseEnterLocal;

    m_onMouseExitLocal = [this](Event* e, Entity* ntt) -> void {
      SetMaterialVal(GetButtonMaterialVal());
    };
    m_onMouseExit = m_onMouseExitLocal;
  }

  void Button::ParameterConstructor()
  {
    // Update surface params.
    ParamMaterial().m_exposed     = false;
    ParamSize().m_category        = ButtonCategory;
    ParamPivotOffset().m_category = ButtonCategory;

    // Define button params.
    ButtonMaterial_Define(GetMaterialManager()->GetCopyOfUIMaterial(),
                          ButtonCategory.Name,
                          ButtonCategory.Priority,
                          true,
                          true);

    HoverMaterial_Define(GetMaterialManager()->GetCopyOfUIMaterial(),
                         ButtonCategory.Name,
                         ButtonCategory.Priority,
                         true,
                         true);
  }

  void Button::ParameterEventConstructor()
  {
    // Always rewire events for correctness.
    Surface::ParameterEventConstructor();

    ParamButtonMaterial().m_onValueChangedFn.push_back(
        [this](Value& oldVal, Value& newVal) -> void {
          // Override surface material.
          SetMaterialVal(std::get<MaterialPtr>(newVal));
        });

    GetMeshComponent()->ParamMesh().m_exposed       = false;
    GetMeshComponent()->ParamCastShadow().m_exposed = false;
  }

} // namespace ToolKit
