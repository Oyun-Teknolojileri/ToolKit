#include "App.h"
#include "SDL.h"
#include "Viewport.h"
#include "PluginManager.h"
#include "Util.h"
#include "DebugNew.h"

namespace ToolKit
{

  void Game::Init(ToolKit::Main* master)
  {
    m_main = master;

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
  }

  void Game::Frame(float deltaTime, Viewport* viewport)
  {
    // Update engine bindings.
    m_viewport = viewport;
    m_scene = GetScene();

    if (!m_scene)
    {
      m_main->m_pluginManager.m_reporterFn("There is not a current scene. Skipping the frame.");
      return;
    }

    const EntityRawPtrArray& entities = m_scene->GetEntities();
    for (Entity* ntt : entities)
    {
      if (ntt->IsDrawable())
      {
        Drawable* dw = static_cast<Drawable*> (ntt);
        m_main->m_renderer.Render(dw, viewport->m_camera, m_sceneLights);
      } 
    }

    IconAnim(deltaTime);
  }

  void Game::Resize(int width, int height)
  {
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
    if (SceneManager* sceneMgr = &m_main->m_sceneManager)
    {
      scene = sceneMgr->m_currentScene;
    }

    return scene;
  }

  void Game::CheckPlayerMove()
  {
    EntityRawPtrArray playerTags = m_scene->GetByTag("player");
    if (playerTags.empty())
    {
      return;
    }

    Entity* player = playerTags.front();
    Vec3 pCenter = player->m_node->GetTranslation(TransformationSpace::TS_WORLD);

    EntityIdArray ignoreList;
    EntityRawPtrArray icons = m_scene->GetByTag("icn");
    ToEntityIdArray(ignoreList, icons);

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

  void Game::IconAnim(float deltaTime)
  {
    static float elapsedTime = 0.0f;
    static float distSign = 1.0f;

    // Animation parameters.
    const float animTime = 500.0f;
    const float totalDist = 1.0f;
    const float totalRotation = 900.0f;

    // Derived constants.
    float lineerSpeed = totalDist / animTime;
    float angularSpeed = glm::radians(totalRotation / animTime);

    if (elapsedTime < animTime)
    {
      EntityRawPtrArray icons = m_scene->GetByTag("icn");
      for (Entity* icn : icons)
      {
        icn->m_node->Translate(Vec3(lineerSpeed * distSign));
        icn->m_node->Rotate(glm::angleAxis(angularSpeed, Y_AXIS));
      }

      elapsedTime += deltaTime;
    }
    else
    {
      elapsedTime = 0.0f;
      distSign *= -1.0f;
    }
  }

}