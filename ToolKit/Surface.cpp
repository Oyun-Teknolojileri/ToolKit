#include "Surface.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Node.h"
#include "Viewport.h"
#include "ToolKit.h"
#include "DebugNew.h"

namespace ToolKit
{
  Surface::Surface()
  {
    ComponentConstructor();
    ParameterConstructor();
    ParameterEventConstructor();
  }

  Surface::Surface(TexturePtr texture, const Vec2& pivotOffset)
    : Surface()
  {
    GetMeshComponent()->Mesh()->m_material->m_diffuseTexture = texture;
    PivotOffset() = pivotOffset;
    SetSizeFromTexture();
    CreateQuat();
  }

  Surface::Surface(TexturePtr texture, const SpriteEntry& entry)
    : Surface()
  {
    GetMeshComponent()->Mesh()->m_material->m_diffuseTexture = texture;
    SetSizeFromTexture();
    CreateQuat(entry);
  }

  Surface::Surface(const String& textureFile, const Vec2& pivotOffset)
    : Surface()
  {
    GetMeshComponent()->Mesh()->m_material->m_diffuseTexture =
      GetTextureManager()->Create<Texture>(textureFile);

    PivotOffset() = pivotOffset;
    SetSizeFromTexture();
    CreateQuat();
  }

  Surface::Surface(const Vec2& size, const Vec2& offset)
    : Surface()
  {
    Size() = size;
    PivotOffset() = offset;
    CreateQuat();
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
  }

  void Surface::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    ParameterEventConstructor();
    CreateQuat();
  }

  void Surface::UpdateGeometry(bool byTexture)
  {
    if (byTexture)
    {
      SetSizeFromTexture();
    }

    MeshPtr mesh = GetMeshComponent()->Mesh();
    mesh->UnInit();
    CreateQuat();
    mesh->Init();
  }

  void Surface::ComponentConstructor()
  {
    MeshComponent* meshComponent = new MeshComponent();
    meshComponent->m_localData[meshComponent->MeshIndex()].m_exposed = false;
    AddComponent(meshComponent);

    MaterialComponent* materialComponent = new MaterialComponent();
    materialComponent->m_localData
    [
      materialComponent->MaterialIndex()
    ].m_exposed = false;

    MaterialPtr material = GetMaterialManager()->GetCopyOfUnlitMaterial();
    material->UnInit();
    material->GetRenderState()->blendFunction =
      BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
    material->GetRenderState()->depthTestEnabled = true;

    materialComponent->Material() = material;

    AddComponent(materialComponent);
  }

  void Surface::ParameterConstructor()
  {
    Size_Define
    (
      { 150.0f, 50.0f },
      SurfaceCategory.Name,
      SurfaceCategory.Priority,
      true,
      true
    );

    PivotOffset_Define
    (
      { 0.5f, 0.5f },
      SurfaceCategory.Name,
      SurfaceCategory.Priority,
      true,
      true
    );

    Material_Define
    (
      GetMaterialComponent()->Material(),
      SurfaceCategory.Name,
      SurfaceCategory.Priority,
      true,
      true
    );
  }

  void Surface::ParameterEventConstructor()
  {
    m_localData[Size_Index].m_onValueChangedFn =
      [this](Value& oldVal, Value& newVal) -> void
    {
      Size() = std::get<Vec2>(newVal);
      UpdateGeometry(false);
    };

    m_localData[PivotOffset_Index].m_onValueChangedFn =
      [this](Value& oldVal, Value& newVal) -> void
    {
      PivotOffset() = std::get<Vec2>(newVal);
      UpdateGeometry(false);
    };

    m_localData[Material_Index].m_onValueChangedFn =
      [this](Value& oldVal, Value& newVal) -> void
    {
      GetMaterialComponent()->Material() = std::get<MaterialPtr>(newVal);
      UpdateGeometry(true);
    };
  }

  Entity* Surface::CopyTo(Entity* copyTo) const
  {
    Entity::CopyTo(copyTo);
    Surface* cpy = static_cast<Surface*> (copyTo);
    return cpy;
  }

  void Surface::ResetCallbacks()
  {
    m_onMouseEnter = nullptr;
    m_onMouseExit = nullptr;
    m_onMouseOver = nullptr;
    m_onMouseClick = nullptr;
  }

  void Surface::CreateQuat()
  {
    float width = Size().x;
    float height = Size().y;
    float depth = 0;
    Vec2 absOffset = Vec2(PivotOffset().x * width, PivotOffset().y * height);

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

    MeshPtr mesh = GetMeshComponent()->Mesh();
    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
  }

  void Surface::CreateQuat(const SpriteEntry& val)
  {
    assert(false && "Old function needs to be re factored.");

    MeshPtr& mesh = GetMeshComponent()->Mesh();
    float imageWidth = static_cast<float>
    (
      mesh->m_material->m_diffuseTexture->m_width
    );

    float imageHeight = static_cast<float>
    (
      mesh->m_material->m_diffuseTexture->m_height
    );

    Rect<float> textureRect;
    textureRect.x = static_cast<float> (val.rectangle.x) /
    static_cast<float> (imageWidth);

    textureRect.height =
    (
      static_cast<float> (val.rectangle.height) /
      static_cast<float> (imageHeight)
    );

    textureRect.y = 1.0f -
    (
      static_cast<float> (val.rectangle.y) / static_cast<float> (imageHeight)
    ) - textureRect.height;

    textureRect.width = static_cast<float> (val.rectangle.width) /
      static_cast<float> (imageWidth);

    float depth = 0.0f;
    float width = static_cast<float> (val.rectangle.width);
    float height = static_cast<float> (val.rectangle.height);
    Vec2 absOffset = Vec2
    (
      val.offset.x * val.rectangle.width,
      val.offset.y * val.rectangle.height
    );

    VertexArray vertices;
    vertices.resize(6);
    vertices[0].pos = Vec3(-absOffset.x, -absOffset.y, depth);
    vertices[0].tex = Vec2(textureRect.x, 1.0f - textureRect.y);
    vertices[1].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[1].tex = Vec2
    (
      textureRect.x + textureRect.width,
      1.0f - textureRect.y
    );

    vertices[2].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[2].tex = Vec2
    (
      textureRect.x,
      1.0f - (textureRect.y + textureRect.height)
    );

    vertices[3].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[3].tex = Vec2
    (
      textureRect.x + textureRect.width,
      1.0f - textureRect.y
    );

    vertices[4].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
    vertices[4].tex = Vec2
    (
      textureRect.x + textureRect.width,
      1.0f - (textureRect.y + textureRect.height)
    );

    vertices[5].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[5].tex = Vec2
    (
      textureRect.x,
      1.0f - (textureRect.y + textureRect.height)
    );

    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
  }

  void Surface::SetSizeFromTexture()
  {
    Size() =
    Vec2
    (
      Material()->m_diffuseTexture->m_width,
      Material()->m_diffuseTexture->m_height
    );
  }

  // Button
  //////////////////////////////////////////

  Button::Button()
  {
    ResetCallbacks();
  }

  Button::Button(const Vec2& size)
    : Surface(size)
  {
    ResetCallbacks();
  }

  Button::Button
  (
    const TexturePtr& buttonImage,
    const TexturePtr& mouseOverImage
  )
    : Surface(buttonImage, Vec2())
  {
    m_buttonImage = buttonImage;
    m_mouseOverImage = mouseOverImage;
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
    if (!m_mouseOverImage && !m_buttonImage)
    {
      return;
    }

    XmlNode* node = doc->allocate_node(rapidxml::node_element, "Button");
    if (m_mouseOverImage)
    {
      String relPath = GetRelativeResourcePath(m_mouseOverImage->GetFile());
      node->append_attribute
      (
        doc->allocate_attribute
        (
          "mouseOverImage",
          doc->allocate_string(relPath.c_str())
        )
      );
    }

    if (m_buttonImage)
    {
      String relPath = GetRelativeResourcePath(m_buttonImage->GetFile());
      node->append_attribute
      (
        doc->allocate_attribute
        (
          "buttonImage",
          doc->allocate_string(relPath.c_str())
        )
      );
    }

    parent->last_node()->append_node(node);
  }

  void Button::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Surface::DeSerialize(doc, parent);
    if (XmlNode* buttonNode = parent->first_node("Button"))
    {
      String image;
      ReadAttr(buttonNode, "mouseOverImage", image);
      if (!image.empty())
      {
        String path = TexturePath(image);
        NormalizePath(path);
        m_mouseOverImage = GetTextureManager()->Create<Texture>(path);
      }

      ReadAttr(buttonNode, "buttonImage", image);
      if (!image.empty())
      {
        String path = TexturePath(image);
        NormalizePath(path);
        m_buttonImage = GetTextureManager()->Create<Texture>(path);
      }
    }
  }

  void Button::ResetCallbacks()
  {
    Surface::ResetCallbacks();

    m_onMouseEnterLocal = [this](Event* e, Entity* ntt) -> void
    {
      GetMeshComponent()->Mesh()->
        m_material->m_diffuseTexture = m_mouseOverImage;
    };
    m_onMouseEnter = m_onMouseEnterLocal;

    m_onMouseExitLocal = [this](Event* e, Entity* ntt) -> void
    {
      GetMeshComponent()->Mesh()->
        m_material->m_diffuseTexture = m_buttonImage;
    };
    m_onMouseExit = m_onMouseExitLocal;
  }

}  // namespace ToolKit

