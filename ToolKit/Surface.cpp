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
    MaterialPtr mat = GetMaterialManager()->GetCopyOfUnlitMaterial();
    mat->UnInit();
    mat->GetRenderState()->blendFunction =
      BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;

    mat->GetRenderState()->depthTestEnabled = true;
    GetMesh()->m_material = mat;
  }

  Surface::Surface(TexturePtr texture, const Vec2& pivotOffset)
    : Surface()
  {
    GetMesh()->m_material->m_diffuseTexture = texture;
    m_pivotOffset = pivotOffset;
    SetSizeFromTexture();
    CreateQuat();
  }

  Surface::Surface(TexturePtr texture, const SpriteEntry& entry)
    : Surface()
  {
    GetMesh()->m_material->m_diffuseTexture = texture;
    SetSizeFromTexture();
    CreateQuat(entry);
  }

  Surface::Surface(const String& textureFile, const Vec2& pivotOffset)
    : Surface()
  {
    GetMesh()->m_material->m_diffuseTexture =
      GetTextureManager()->Create<Texture>(textureFile);

    m_pivotOffset = pivotOffset;
    SetSizeFromTexture();
    CreateQuat();
  }

  Surface::Surface(const Vec2& size, const Vec2& offset)
    : Surface()
  {
    m_size = size;
    m_pivotOffset = offset;
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
    XmlNode* nttNode = parent->last_node();
    XmlNode* prop = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "Size"
    );
    nttNode->append_node(prop);
    WriteVec(prop, doc, m_size);

    prop = doc->allocate_node
    (
      rapidxml::node_type::node_element,
      "Offset"
    );
    nttNode->append_node(prop);
    WriteVec(prop, doc, m_pivotOffset);

    MeshPtr mesh = GetMesh();
    String matFile = mesh->m_material->GetFile();
    if (matFile.empty())
    {
      mesh->m_material->m_name = NameC();
    }
    mesh->m_material->Save(true);
    WriteMaterial(nttNode, doc, matFile);
  }

  void Surface::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    XmlNode* prop = parent->first_node("Size");
    ReadVec<Vec2>(prop, m_size);
    prop = parent->first_node("Offset");
    ReadVec<Vec2>(prop, m_pivotOffset);
    CreateQuat();
    GetMesh()->m_material = ReadMaterial(parent);
  }

  void Surface::UpdateGeometry(bool byTexture)
  {
    if (byTexture)
    {
      SetSizeFromTexture();
    }

    MeshPtr mesh = GetMesh();
    mesh->UnInit();
    CreateQuat();
    mesh->Init();
  }

  Entity* Surface::CopyTo(Entity* copyTo) const
  {
    Drawable::CopyTo(copyTo);
    Surface* cpy = static_cast<Surface*> (copyTo);
    cpy->m_size = m_size;
    cpy->m_pivotOffset = m_pivotOffset;

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
    float width = m_size.x;
    float height = m_size.y;
    float depth = 0;
    Vec2 absOffset = Vec2(m_pivotOffset.x * width, m_pivotOffset.y * height);

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

    MeshPtr mesh = GetMesh();
    mesh->m_clientSideVertices = vertices;
    mesh->CalculateAABB();
  }

  void Surface::CreateQuat(const SpriteEntry& val)
  {
    MeshPtr& mesh = GetMesh();
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
    MeshPtr& mesh = GetMesh();
    m_size =
    {
      mesh->m_material->m_diffuseTexture->m_width,
      mesh->m_material->m_diffuseTexture->m_height
    };
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
      GetMesh()->m_material->m_diffuseTexture = m_mouseOverImage;
    };
    m_onMouseEnter = m_onMouseEnterLocal;

    m_onMouseExitLocal = [this](Event* e, Entity* ntt) -> void
    {
      GetMesh()->m_material->m_diffuseTexture = m_buttonImage;
    };
    m_onMouseExit = m_onMouseExitLocal;
  }

}  // namespace ToolKit

