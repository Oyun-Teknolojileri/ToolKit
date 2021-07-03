#pragma once

#include "Drawable.h"
#include "Mesh.h"
#include "Ship.h"
#include "Node.h"
#include <array>
#include "DebugNew.h"

using namespace ToolKit;

class Projectile : public Drawable
{
public:
  Projectile(Vec3 pos)
  {
    m_mesh = GetMeshManager()->Create<Mesh>(MeshPath("projectile.mesh"));
    m_node->SetTranslation(pos);
  }
};

class ProjectileManager
{
public:
  ProjectileManager()
  {
  }

  ~ProjectileManager()
  {
    for (size_t i = 0; i < m_projectiles.size(); i++)
    {
      SafeDel(m_projectiles[i]);
    }
  }

  void FireProjectile(Vec3 pos)
  {
    Projectile* projectile = new Projectile(pos);
    m_projectiles.push_back(projectile);
  }

  void UpdateProjectiles()
  {
    for (int i = (int)m_projectiles.size() - 1; i > -1; i--)
    {
      Projectile* projectile = m_projectiles[i];
      projectile->m_node->Translate({ 0.0f, 0.0f, -2.1f });

      if (projectile->m_node->GetTranslation(TransformationSpace::TS_WORLD).z <= m_projectileDethZone)
      {
        m_projectiles.erase(m_projectiles.begin() + i);
        SafeDel(projectile);
      }
    }
  }

public:
  int m_projectileDethZone = -15;
  std::vector<Projectile*> m_projectiles;
};
