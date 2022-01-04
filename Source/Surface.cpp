#include "stdafx.h"
#include "Surface.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "Node.h"
#include "Viewport.h"
#include "DebugNew.h"

namespace ToolKit
{
  Surface::Surface()
  {
    MaterialPtr mat = GetMaterialManager()->GetCopyOfUnlitMaterial();
    mat->UnInit();
    mat->GetRenderState()->blendFunction = BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
    mat->GetRenderState()->depthTestEnabled = true;
    m_mesh->m_material = mat;
  }

  Surface::Surface(TexturePtr texture, const Vec2& pivotOffset)
    : Surface()
  {
    m_mesh->m_material->m_diffuseTexture = texture;
    m_pivotOffset = pivotOffset;
    SetSizeFromTexture();
    CreateQuat();
  }

  Surface::Surface(TexturePtr texture, const SpriteEntry& entry)
    : Surface()
  {
    m_mesh->m_material->m_diffuseTexture = texture;
    SetSizeFromTexture();
    CreateQuat(entry);
  }

  Surface::Surface(const String& textureFile, const Vec2& pivotOffset)
    : Surface()
  {
    m_mesh->m_material->m_diffuseTexture = GetTextureManager()->Create<Texture>(textureFile);
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

    if (m_mesh->m_material->m_file.empty())
    {
      m_mesh->m_material->m_name = m_name;
    }
    m_mesh->m_material->Save(true);
    WriteMaterial(nttNode, doc, m_mesh->m_material->m_file);
  }

  void Surface::DeSerialize(XmlDocument* doc, XmlNode* parent)
  {
    Entity::DeSerialize(doc, parent);
    XmlNode* prop = parent->first_node("Size");
    ReadVec<Vec2>(prop, m_size);
    prop = parent->first_node("Offset");
    ReadVec<Vec2>(prop, m_pivotOffset);
    CreateQuat();
    m_mesh->m_material = ReadMaterial(parent);
  }

  void Surface::UpdateGeometry(bool byTexture)
  {
    if (byTexture)
    {
      SetSizeFromTexture();
    }

    m_mesh->UnInit();
    CreateQuat();
    m_mesh->Init();
  }

  Entity* Surface::GetCopy(Entity* copyTo) const
  {
    Drawable::GetCopy(copyTo);
    Surface* cpy = static_cast<Surface*> (copyTo);
    cpy->m_size = m_size;
    cpy->m_pivotOffset = m_pivotOffset;

    return cpy;
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

    m_mesh->m_clientSideVertices = vertices;
    m_mesh->CalculateAABB();
  }

  void Surface::CreateQuat(const SpriteEntry& val)
  {
    float imageWidth = (float)m_mesh->m_material->m_diffuseTexture->m_width;
    float imageHeight = (float)m_mesh->m_material->m_diffuseTexture->m_height;

    Rect<float> textureRect;
    textureRect.x = (float)val.rectangle.x / (float)imageWidth;
    textureRect.height = ((float)val.rectangle.height / (float)imageHeight);
    textureRect.y = 1.0f - ((float)val.rectangle.y / (float)imageHeight) - textureRect.height;
    textureRect.width = (float)val.rectangle.width / (float)imageWidth;

    float depth = 0.0f;
    float width = (float)val.rectangle.width;
    float height = (float)val.rectangle.height;
    Vec2 absOffset = Vec2(val.offset.x * val.rectangle.width, val.offset.y * val.rectangle.height);

    VertexArray vertices;
    vertices.resize(6);
    vertices[0].pos = Vec3(-absOffset.x, -absOffset.y, depth);
    vertices[0].tex = Vec2(textureRect.x, 1.0f - textureRect.y);
    vertices[1].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[1].tex = Vec2(textureRect.x + textureRect.width, 1.0f - textureRect.y);
    vertices[2].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[2].tex = Vec2(textureRect.x, 1.0f - (textureRect.y + textureRect.height));

    vertices[3].pos = Vec3(width - absOffset.x, -absOffset.y, depth);
    vertices[3].tex = Vec2(textureRect.x + textureRect.width, 1.0f - textureRect.y);
    vertices[4].pos = Vec3(width - absOffset.x, height - absOffset.y, depth);
    vertices[4].tex = Vec2(textureRect.x + textureRect.width, 1.0f - (textureRect.y + textureRect.height));
    vertices[5].pos = Vec3(-absOffset.x, height - absOffset.y, depth);
    vertices[5].tex = Vec2(textureRect.x, 1.0f - (textureRect.y + textureRect.height));

    m_mesh->m_clientSideVertices = vertices;
    m_mesh->CalculateAABB();
  }

  void Surface::SetSizeFromTexture()
  {
    m_size =
    {
      m_mesh->m_material->m_diffuseTexture->m_width,
      m_mesh->m_material->m_diffuseTexture->m_height
    };
  }

  // SurfaceObserver
  //////////////////////////////////////////

  void SurfaceObserver::SetRoot(Entity* root)
  {
    m_root = root;
  }

  void SurfaceObserver::Update(float deltaTimeSec, Viewport* vp)
  {
    EntityRawPtrArray entities;
    GetChildren(m_root, entities);

    const EventPool& events = Main::GetInstance()->m_eventPool;
    if (entities.empty() || events.empty())
    {
      return;
    }

    for (Entity* ntt : entities)
    {
      // Process events.
      for (Event* e : events)
      {
        if (ntt->GetType() == EntityType::Entity_Surface)
        {
          Surface* surface = static_cast<Surface*> (ntt);

          if (surface->m_onMouseOver)
          {
            if (CheckMouseOver(surface, e, vp))
            {
              surface->m_onMouseOver(e, ntt);
            }
          }

          if (surface->m_onMouseClick)
          {
            if (CheckMouseClick(surface, e, vp))
            {
              surface->m_onMouseClick(e, ntt);
            }
          }

          if (surface->m_onMouseEnter)
          {
            if (CheckMouseEnter(surface, e, vp))
            {
              surface->m_onMouseEnter(e, ntt);
            }
          }

          if (surface->m_onMouseExit)
          {
            if (CheckMouseExit(surface, e, vp))
            {
              surface->m_onMouseExit(e, ntt);
            }
          }

          // Update post states.
          surface->m_mouseOverPrev = CheckMouseOver(surface, e, vp);
        }
      }
    }
  }

  bool SurfaceObserver::CheckMouseClick(Surface* surface, Event* e, Viewport* vp)
  {
    if (CheckMouseOver(surface, e, vp))
    {
      MouseEvent* me = static_cast<MouseEvent*> (e);
      return me->m_action == EventAction::LeftClick;
    }

    return false;
  }

  bool SurfaceObserver::CheckMouseOver(Surface* surface, Event* e, Viewport* vp)
  {
    if (e->m_type == Event::EventType::Mouse)
    {
      BoundingBox box = surface->GetAABB(true);
      Ray ray = vp->RayFromMousePosition();

      float t = 0.0f;
      if (RayBoxIntersection(ray, box, t))
      {
        return true;
      }
    }

    return false;
  }

  bool SurfaceObserver::CheckMouseEnter(Surface* surface, Event* e, Viewport* vp)
  {
    if (!surface->m_mouseOverPrev)
    {
      if (CheckMouseOver(surface, e, vp))
      {
        return true;
      }
    }

    return false;
  }

  bool SurfaceObserver::CheckMouseExit(Surface* surface, Event* e, Viewport* vp)
  {
    if (surface->m_mouseOverPrev)
    {
      if (!CheckMouseOver(surface, e, vp))
      {
        return true;
      }
    }

    return false;
  }

}
