#include "App.h"
#include "SDL.h"
#include "Viewport.h"
#include "PluginManager.h"
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

    Ray ray = m_viewport->RayFromMousePosition();
    Scene::PickData pd = m_scene->PickObject(ray);
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

}