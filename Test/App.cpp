#include "stdafx.h"
#include "App.h"
#include "SDL.h"
#include "Viewport.h"
#include "PluginManager.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  void Game::Init(Main* master)
  {
    Main::SetProxy(master);

    // Lights and camera.
    m_lightMaster = new Node();

    Light* light = new Light();
    light->Yaw(glm::radians(-45.0f));
    m_lightMaster->AddChild(light->m_node);
    m_sceneLights.push_back(light);

    light = new Light();
    light->m_intensity = 0.5f;
    light->Yaw(glm::radians(60.0f));
    m_lightMaster->AddChild(light->m_node);
    m_sceneLights.push_back(light);

    light = new Light();
    light->m_intensity = 0.3f;
    light->Yaw(glm::radians(-140.0f));
    m_lightMaster->AddChild(light->m_node);
    m_sceneLights.push_back(light);

    m_cam = new Camera();
    m_cam->SetLens(glm::half_pi<float>(), 10, 10, 10, 10, 1, 1000);
    m_cam->m_node->SetTranslation({ 5.0f, 5.0f, 5.0f });
    m_cam->LookAt({ 0.0f, 0.0f, 0.0f });
    m_cam->m_node->AddChild(m_lightMaster);

    // try loading anim
    m_anim = new Animation(AnimationPath(ConcatPaths({ "anim", "TorusAction.anim" })));
    m_anim->Load();
    m_anim->m_loop = true;
    EntityRawPtrArray root = GetScene()->GetByTag("anim");
    if (!root.empty())
    {
      Entity* ntt = root.front();
      GetAnimationPlayer()->AddRecord(ntt, m_anim);
    }
  }

  void Game::Destroy()
  {
    for (Entity* ntt : m_sceneLights)
    {
      delete ntt;
    }

    m_sceneLights.clear();
    SafeDel(m_cam);
    SafeDel(m_lightMaster);

    delete this;
  }

  void Game::Frame(float deltaTime, Viewport* viewport)
  {
    // Update engine bindings.
    m_viewport = viewport;
    m_scene = GetScene();

    if (!m_scene)
    {
      return;
    }

    const EntityRawPtrArray& entities = m_scene->GetEntities();
    for (Entity* ntt : entities)
    {
      if (ntt->IsDrawable())
      {
        Drawable* dw = static_cast<Drawable*> (ntt);
        GetRenderer()->Render(dw, viewport->m_camera, m_sceneLights);
      } 
    }

    IconAnim(deltaTime);
    if (!m_gameOver)
    {
      CheckPickups();
      CheckEnemyMove();
    }
  }

  void Game::Event(SDL_Event event)
  {
    switch (event.type)
    {
    case SDL_MOUSEBUTTONDOWN:
      if (event.button.button == SDL_BUTTON_LEFT)
      {
        CheckPlayerMove();
      }
      break;
    }
  }

  ScenePtr Game::GetScene()
  {
    ScenePtr scene = nullptr;
    if (SceneManager* sceneMgr = GetSceneManager())
    {
      scene = sceneMgr->m_currentScene;
    }

    return scene;
  }

  Entity* Game::GetPlayer()
  {
    EntityRawPtrArray playerTags = m_scene->GetByTag("player");
    if (playerTags.empty())
    {
      return nullptr;
    }

    return playerTags.front();
  }

  void Game::CheckPlayerMove()
  {
    if (m_scene == nullptr)
    {
      return;
    }

    Entity* player = GetPlayer();
    Vec3 pCenter = player->m_node->GetTranslation(TransformationSpace::TS_WORLD);

    EntityIdArray ignoreList;
    EntityRawPtrArray icons = m_scene->GetByTag("pickup");
    EntityRawPtrArray foos = m_scene->GetByTag("foo");
    ToEntityIdArray(ignoreList, foos);

    ignoreList.push_back(player->m_id);

    Ray ray = m_viewport->RayFromMousePosition();
    Scene::PickData pd = m_scene->PickObject(ray, ignoreList);
    if (pd.entity)
    {
      if (pd.entity->m_tag == "grnd")
      {        
        Quaternion rot = player->m_node->GetOrientation();
        Vec3 fdir = glm::normalize(rot * Z_AXIS);
        Vec3 rdir = glm::normalize(glm::cross(fdir, Y_AXIS));

        Drawable* ground = static_cast<Drawable*> (pd.entity);
        BoundingBox bb = ground->GetAABB(true);
        float dia = glm::compMax(bb.max - bb.min);
        Vec3 playerPos = player->m_node->GetTranslation();
        if (glm::distance(playerPos, bb.GetCenter()) > dia)
        {
          return;
        }

        Vec3 target = glm::normalize(bb.GetCenter() - playerPos);

        float front = glm::dot(fdir, target);
        float right = glm::dot(rdir, target); 
        
        float angle = 0.0f;
        if (glm::epsilonEqual(front, 1.0f, 0.001f))
        {
          angle = 0.0f;
        }
        else if (front < -0.001f)
        {
          angle = glm::pi<float>();
        }

        if (glm::epsilonEqual(right, 1.0f, 0.001f))
        {
          angle = -glm::half_pi<float>();
        }
        else if (right < -0.001f)
        {
          angle = glm::half_pi<float>();
        }

        Quaternion q = glm::angleAxis(angle, Y_AXIS);
        player->m_node->Rotate(q);

        // Move player.
        player->m_node->SetTranslation(bb.GetCenter(), TransformationSpace::TS_WORLD);
        BoundingBox pb = player->GetAABB(true);
        EntityRawPtrArray foos = m_scene->GetByTag("foo");
        for (Entity* foo : foos)
        {
          BoundingBox fb = foo->GetAABB(true);
          if (BoxBoxIntersection(fb, pb))
          {
            foo->m_node->Translate(Y_AXIS * 2.0f);
          }
        }
      }
    }
  }

  void Game::CheckEnemyMove()
  {
    EntityRawPtrArray enemies = m_scene->GetByTag("foo");
    for (Entity* foo : enemies)
    {
      BoundingBox fb = GetForwardBB(foo);
      if (Entity* player = GetPlayer())
      {
        BoundingBox pb = player->GetAABB(true);
        if (BoxBoxIntersection(fb, pb))
        {
          Vec3 fdir = GetForwardDir(foo);
          foo->m_node->Translate(fdir * m_blockSize);
          player->m_node->Translate(Y_AXIS * 2.0f);
          m_gameOver = true;
          return;
        }
      }
    }
  }

  void Game::CheckPickups()
  {
    EntityRawPtrArray pickups = m_scene->GetByTag("pickup");
    EntityRawPtrArray playerTags = m_scene->GetByTag("player");
    if (playerTags.empty())
    {
      return;
    }
    Entity* player = playerTags.front();

    for (Entity* pickup : pickups)
    {
      BoundingBox a = pickup->GetAABB(true);
      BoundingBox b = player->GetAABB(true);
      if (BoxBoxIntersection(a, b))
      {
        GetScene()->RemoveEntity(pickup->m_id);
        SafeDel(pickup);
      }
    }
  }

  void Game::IconAnim(float deltaTime)
  {
    static float elapsedTime = 0.0f;

    // Animation parameters.
    const float animTime = 1000.0f; // Time unit is milli seconds.

    const float upperLimit = 2.0f;
    const float lowerLimit = 0.65f;

    const float totalDist = upperLimit - lowerLimit;
    const float totalRotation = 180.0f;

    // Derived constants.
    float lineerSpeed = totalDist / animTime;
    float angularSpeed = glm::radians(totalRotation / animTime);

    static std::unordered_map<EntityId, float> switches;

    if (elapsedTime < animTime)
    {
      EntityRawPtrArray icons = m_scene->GetByTag("pickup");
      for (Entity* icn : icons)
      {
        Vec3 ts = icn->m_node->GetTranslation();
        if (switches.find(icn->m_id) == switches.end())
        {
          switches[icn->m_id] = 1.0f;
        }

        if (ts.y > upperLimit)
        {
          switches[icn->m_id] = -1.0f;
        }
        if (ts.y < lowerLimit)
        {
          switches[icn->m_id] = 1.0f;
        }

        float delta = deltaTime * lineerSpeed * switches[icn->m_id];
        icn->m_node->Translate(Vec3(0.0f, delta, 0.0f));
        delta = deltaTime * angularSpeed;
        icn->m_node->Rotate(glm::angleAxis(delta, Y_AXIS));
      }

      elapsedTime += deltaTime;
    }
    else
    {
      elapsedTime = 0.0f;
    }
  }

  BoundingBox Game::GetForwardBB(Entity* ntt)
  {
    Vec3 fdir = GetForwardDir(ntt);
    fdir = glm::round(fdir);
    BoundingBox fb = ntt->GetAABB(true);

    Mat4 next = glm::translate(Mat4(), fdir);
    TransformAABB(fb, next);

    return fb;
  }

  Vec3 Game::GetForwardDir(Entity* ntt)
  {
    Mat4 ts = ntt->m_node->GetTransform();
    Vec3 fdir = glm::normalize(glm::column(ts, 2));
    return fdir;
  }

}