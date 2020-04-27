#pragma once

#include "Drawable.h"
#include "Mesh.h"
#include "ToolKit.h"
#include "Node.h"
#include "glm\gtc\random.hpp"

using namespace ToolKit;

class Meteor : public ToolKit::Drawable
{
public:

  Meteor()
  {
    m_mesh = ToolKit::GetMeshManager()->Create(ToolKit::MeshPath("rock.mesh"));
  }

  float m_collisionRadius = 1.3f;
  float m_speed = 0.3f;
};

class MeteorManager
{
public:

  ~MeteorManager()
  {
    for (auto entry : m_meteors)
      SafeDel(entry);
  }

  void Spawn(bool speedy = false)
  {
    Meteor* meteor = new Meteor();
    if (speedy)
    {
      meteor->m_speed = 0.8f;
    }

    Vec3 wt = Vec3(glm::linearRand(-10.0f, 10.0f), 0.0f, glm::linearRand(-45.0f, -10.0f) - 25.0f);
    meteor->m_node->SetTranslation(wt, TransformationSpace::TS_WORLD);
    Quaternion wo = glm::angleAxis(glm::linearRand(-90.0f, 90.0f), glm::sphericalRand(1.0f));
    meteor->m_node->SetOrientation(wo, TransformationSpace::TS_WORLD);

    for (auto entry : m_meteors)
    {
      if (SphereSphereIntersection(
        entry->m_node->GetTranslation(TransformationSpace::TS_WORLD),
        entry->m_collisionRadius + 1,
        meteor->m_node->GetTranslation(TransformationSpace::TS_WORLD),
        meteor->m_collisionRadius))
      {
        SafeDel(meteor);
        return;
      }
    }

    m_meteors.push_back(meteor);
  }

  void Update(int& score)
  {
    for (int i = (int)m_meteors.size() - 1; i > -1; i--)
    {
      m_meteors[i]->m_node->Translate({ 0.0f, 0.0f, m_meteors[i]->m_speed });
      m_meteors[i]->m_node->Rotate(glm::angleAxis(glm::radians(glm::linearRand(0.1f, 1.5f)), ToolKit::Z_AXIS), TransformationSpace::TS_LOCAL);
      if (m_meteors[i]->m_node->GetTranslation(TransformationSpace::TS_WORLD).z > 20)
      {
        SafeDel(m_meteors[i]);
        m_meteors.erase(m_meteors.begin() + i);
        score -= 10;
        if (score < 0)
          score = 0;
      }
    }
  }

  std::vector<Meteor*> m_meteors;
};