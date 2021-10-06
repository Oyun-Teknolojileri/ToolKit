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
    CheckPickups();
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

  void Game::CheckPlayerMove()
  {
    if (m_scene == nullptr)
    {
      return;
    }

    EntityRawPtrArray playerTags = m_scene->GetByTag("player");
    if (playerTags.empty())
    {
      return;
    }

    Entity* player = playerTags.front();
    Vec3 pCenter = player->m_node->GetTranslation(TransformationSpace::TS_WORLD);

    EntityIdArray ignoreList;
    EntityRawPtrArray icons = m_scene->GetByTag("pickup");
    ToEntityIdArray(ignoreList, icons);
    ignoreList.push_back(player->m_id);

    EntityRawPtrArray ntties = m_scene->Filter
    (
      [](Entity* e) -> bool 
      {
        return e->m_name == "grnd_2" && e->m_tag == "grnd";
      }
    );

    if (!ntties.empty())
    {
      ignoreList.push_back(ntties.front()->m_id);
    }

    auto ee = std::find_if(ntties.begin(), ntties.end(), [](Entity* e) -> bool { if (e->m_name == "grnd_2") return true; return false; });

    Ray ray = m_viewport->RayFromMousePosition();
    Scene::PickData pd = m_scene->PickObject(ray, ignoreList);
    if (pd.entity)
    {
      if (pd.entity->m_tag == "grnd")
      {
        Drawable* ground = static_cast<Drawable*> (pd.entity);
        BoundingBox bb = ground->GetAABB(true);
        
        // Move player.
        player->m_node->SetTranslation(bb.GetCenter(), TransformationSpace::TS_WORLD);
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

}