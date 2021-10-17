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

    // Create tracks.
    m_forward = new Animation(AnimationPath(ConcatPaths({ "anim", "player_forward.anim" })));
    m_forward->Load();
    if (Entity* player = GetPlayer())
    {
      m_playerMove = { player, m_forward };

      // Scene copy doesn't preserve enttiy child relation. Fix it.
      if (Entity* playerGround = GetPlayerGround())
      {
        player->m_node->OrphanSelf();
        playerGround->m_node->AddChild(player->m_node);
      }
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

    EntityRawPtrArray root = GetScene()->GetByTag("anim");
    if (!root.empty())
    {
      Entity* ntt = root.front();
      GetAnimationPlayer()->RemoveRecord(ntt, m_forward);
    }

    SafeDel(m_forward);

    delete this;
  }

  void Game::Frame(float deltaTime, Viewport* viewport)
  {
    // Update engine bindings.
    m_viewport = viewport;
    m_scene = GetScene();
    m_onAnim = GetAnimationPlayer()->Exist(m_playerMove) != -1;

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

    if (!m_onAnim)
    {
      UpdateGroundPos();
      if (!m_gameOver)
      {
        CheckEnemyMove();
        CheckPickups();
      }
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
    EntityRawPtrArray playerTags = GetScene()->GetByTag("player");
    if (playerTags.empty())
    {
      return nullptr;
    }

    return playerTags.front();
  }

  Entity* Game::GetPlayerGround()
  {
    EntityRawPtrArray pgArray = GetScene()->GetByTag("pg");
    if (!pgArray.empty())
    {
      return pgArray.front();
    }

    return nullptr;
  }

  void Game::CheckPlayerMove()
  {
    Entity* player = GetPlayer();
    if (player == nullptr)
    {
      return;
    }

    Node* playerGround = player->m_node->m_parent;
    if (playerGround == nullptr)
    {
      return;
    }

    if (m_scene == nullptr || m_onAnim)
    {
      return;
    }

    EntityIdArray ignoreList;
    EntityRawPtrArray foos = m_scene->GetByTag("foo");
    ToEntityIdArray(ignoreList, foos);
    EntityRawPtrArray pickups = m_scene->GetByTag("pickup");
    ToEntityIdArray(ignoreList, pickups);

    ignoreList.push_back(player->m_id);

    Ray ray = m_viewport->RayFromMousePosition();
    Scene::PickData pickData = m_scene->PickObject(ray, ignoreList);
    if (pickData.entity)
    {
      if (pickData.entity->m_tag == "grnd")
      {
        Quaternion currentOrientation = playerGround->GetOrientation();
        Vec3 forwardDir = glm::normalize(currentOrientation * Z_AXIS);
        Vec3 rightDir = glm::normalize(glm::cross(forwardDir, Y_AXIS));

        Drawable* ground = static_cast<Drawable*> (pickData.entity);
        BoundingBox groundBb = ground->GetAABB(true);
        Vec3 pickedGroundPos = groundBb.GetCenter();
        float dia = glm::compMax(groundBb.max - groundBb.min);
        Vec3 playerGroundPos = playerGround->GetTranslation(TransformationSpace::TS_WORLD);
        pickedGroundPos.y = playerGroundPos.y;
        float dist = glm::distance(playerGroundPos, pickedGroundPos);
        if (dist > dia * 1.1f || dist < 0.1f)
        {
          return;
        }

        Vec3 targetDir = glm::normalize(pickedGroundPos - playerGroundPos);
        float front = glm::dot(forwardDir, targetDir);
        float right = glm::dot(rightDir, targetDir); 
        
        float angle = 0.0f;
        if (glm::abs(front) > glm::abs(right))
        {
          // Front back check.
          if (glm::epsilonEqual(front, 1.0f, 0.001f))
          {
            angle = 0.0f;
          }
          else
          {
            angle = glm::pi<float>();
          }
        }
        else
        {
          // Right left check.
          if (glm::epsilonEqual(right, 1.0f, 0.001f))
          {
            angle = -glm::half_pi<float>();
          }
          else
          {
            angle = glm::half_pi<float>();
          }
        }

        Quaternion targetRotation = glm::angleAxis(angle, Y_AXIS);
        playerGround->Rotate(targetRotation);

        // Move player.
        m_playerMove.second->m_state = Animation::State::Play;
        GetAnimationPlayer()->AddRecord(m_playerMove);

        groundBb =
        {
          pickedGroundPos - Vec3(m_blockSize * 0.5f, 0.0f, m_blockSize * 0.5f),
          pickedGroundPos + Vec3(m_blockSize * 0.5f, m_blockSize, m_blockSize * 0.5f)
        };

        EntityRawPtrArray foos = m_scene->GetByTag("foo");
        for (Entity* foo : foos)
        {
          Vec3 fooPos = foo->GetAABB(true).GetCenter();
          if (BoxPointIntersection(groundBb, fooPos))
          {
            foo->m_node->Translate(Y_AXIS * 2.0f);
          }
        }
      }
    }
  }

  void Game::CheckEnemyMove()
  {
    Entity* player = GetPlayer();
    if (player == nullptr)
    {
      return;
    }

    Vec3 playerGround = player->m_node->m_parent->GetTranslation(TransformationSpace::TS_WORLD);
    GetPluginManager()->m_reporterFn("player ground " + glm::to_string(playerGround));
    EntityRawPtrArray enemies = m_scene->GetByTag("foo");
    for (Entity* foo : enemies)
    {
      BoundingBox fooBox = GetForwardBB(foo->m_node);
      GetPluginManager()->m_reporterFn("foo  " + glm::to_string(fooBox.GetCenter()));
      if (BoxPointIntersection(fooBox, playerGround))
      {
        Vec3 fdir = GetForwardDir(foo);
        foo->m_node->Translate(fdir * m_blockSize);
        player->m_node->Translate(Y_AXIS * 2.0f);
        m_gameOver = true;
        return;
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

  BoundingBox Game::GetForwardBB(Node* node)
  {
    Mat4 ts = node->GetTransform(TransformationSpace::TS_WORLD);
    Vec3 pos = glm::column(ts, 3);
    Vec3 dir = glm::column(ts, 2);
    dir = glm::normalize(dir);
    
    pos += dir * m_blockSize;
    pos.y = 0.0f;

    BoundingBox bb =
    {
      pos - Vec3(m_blockSize * 0.5f, 0.0f, m_blockSize * 0.5f),
      pos + Vec3(m_blockSize * 0.5f)
    };
    
    return bb;
  }

  Vec3 Game::GetForwardDir(Entity* ntt)
  {
    Mat4 ts = ntt->m_node->GetTransform();
    Vec3 fdir = glm::normalize(glm::column(ts, 2));
    return fdir;
  }

  void Game::UpdateGroundPos()
  {
    Entity* player = GetPlayer();
    if (player == nullptr)
    {
      return;
    }

    Node* playerGround = player->m_node->m_parent;
    if (playerGround == nullptr)
    {
      return;
    }

    playerGround->Orphan(player->m_node, true);
    Vec3 pos = player->m_node->GetTranslation(TransformationSpace::TS_WORLD);
    pos.y = playerGround->GetTranslation(TransformationSpace::TS_WORLD).y;
    playerGround->SetTranslation(pos);
    playerGround->AddChild(player->m_node, true);
  }

}