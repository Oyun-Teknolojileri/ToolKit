#pragma once

#include "Directional.h"
#include "Renderer.h"
#include "SDL.h"
#include "Mesh.h"
#include "Drawable.h"
#include "Node.h"
#include "Ship.h"
#include "Surface.h"
#include "Common/TTFText.h"
#include "Projectile.h"
#include "SpriteSheet.h"
#include "DebugNew.h"
#include "Meteor.h"
#include "MathUtil.h"
#include "Explosion.h"
#include "Material.h"
#include "PowerUp.h"
#include "Audio.h"
#include "Primative.h"
#include <algorithm>

class App
{
public:

  ~App()
  {
    SafeDel(m_spaceShip);
    SafeDel(m_crosshair);
    SafeDel(m_scoreTxt);
    SafeDel(m_gameOverTxt);
    SafeDel(m_quitReplay);
  }

  void Init()
  {
    srand((int)time(0)); // Seed

    float wide = 15.0f;
    m_cam.SetLens((float)m_windowWidth / (float)m_windowHeight, -wide, wide, -wide, wide, 1.0f, 1000.0f);
    //m_cam.SetLens(glm::radians(90.0f), 640.0f, 768.0f, 1.0f, 100.0f);
    m_cam.Translate(glm::vec3(0.0f, 10.0f, 0.0f));
    m_cam.Pitch(glm::radians(-90.0f));

    m_scoreTxt = new TTFText(ToolKit::FontPath("techno_hideo.ttf"), 45);
    m_scoreTxt->SetText("Score : 0");
    m_scoreTxt->SetColor({ 255, 0, 0 });
    m_scoreTxt->SetPos((int)(m_windowWidth * 150.0f / 640.0f), (int)(m_windowHeight * 700.0f / 768.0f));

    m_gameOverTxt = new TTFText(ToolKit::FontPath("techno_hideo.ttf"), 45);
    m_gameOverTxt->SetText("GAME OVER");
    m_gameOverTxt->SetColor({ 255, 0, 0 });
    m_gameOverTxt->SetPos((int)(m_windowWidth * 110.0f / 640.0f), (int)(m_windowHeight * 400.0f / 768.0f));

    m_quitReplay = new TTFText(ToolKit::FontPath("techno_hideo.ttf"), 45);
    m_quitReplay->SetText("ESC or R");
    m_quitReplay->SetColor({ 255, 0, 0 });
    m_quitReplay->SetPos((int)(m_windowWidth * 179.0f / 640.0f), (int)(m_windowHeight * 300.0f / 768.0f));

    m_spaceShip = new Ship();
    m_spaceShip->m_node->Translate(glm::vec3(4, 0, 4));

    m_crosshair = new ToolKit::Surface(ToolKit::TexturePath("crosshair.png"), glm::vec2(0.5f, 0.5f));
    m_crosshair->Load();

    m_sscp = glm::ivec2(m_windowWidth / 2, m_windowHeight / 2);
    m_crosshair->m_node->Translate(glm::vec3(m_sscp.x, m_sscp.y, 0));

    m_backGround.m_mesh = ToolKit::GetMeshManager()->Create(ToolKit::MeshPath("earthBg.mesh"));
    m_paralaxLayer.m_mesh = ToolKit::GetMeshManager()->Create(ToolKit::MeshPath("starParalaxLayer.mesh"));
    m_paralaxLayer.m_mesh->m_material->GetRenderState()->blendFunction = ToolKit::BlendFunction::SRC_ALPHA_ONE_MINUS_SRC_ALPHA;
    m_paralaxLayer.m_mesh->m_material->GetRenderState()->depthTestEnabled = false;

    m_lazerShotWav = ToolKit::Main::GetInstance()->m_audioMan.Create(ToolKit::AudioPath("lazerShot.wav"));
    m_lazerShotSource.AttachAudio(m_lazerShotWav);
    m_lazerShotSource.SetVolume(0.5f);

    m_explosionWav = ToolKit::Main::GetInstance()->m_audioMan.Create(ToolKit::AudioPath("explosion.wav"));
    m_explosionSource.AttachAudio(m_explosionWav);
    m_explosionSource.SetVolume(0.1f);

    m_shipExplosionWav = ToolKit::Main::GetInstance()->m_audioMan.Create(ToolKit::AudioPath("shipExplosion.wav"));
    m_shipExplosionSource.AttachAudio(m_shipExplosionWav);

    m_ambientLoopWav = ToolKit::Main::GetInstance()->m_audioMan.Create(ToolKit::AudioPath("ambientLoop.wav"));
    m_ambientLoopSource.AttachAudio(m_ambientLoopWav);
    m_ambientLoopSource.SetLoop(true);
    ToolKit::AudioPlayer::Play(&m_ambientLoopSource);
  }

  glm::ivec2 GetSSP(glm::mat4 model)
  {
    glm::mat4 view = m_cam.GetViewMatrix();
    glm::mat4 project = m_cam.GetData().projection;
    glm::vec3 screenPos = glm::project(glm::vec3(), view * model, project, glm::vec4(0.0f, 0.0f, m_windowWidth, m_windowHeight));
    return glm::ivec2(screenPos.x, screenPos.y);
  }

  glm::vec3 GetWSCP()
  {
    glm::vec3 cp = glm::vec3(m_sscp, 0);
    glm::mat4 view = m_cam.GetViewMatrix();
    glm::mat4 project = m_cam.GetData().projection;
    return glm::unProject(cp, view, project, glm::vec4(0.0f, 0.0f, m_windowWidth, m_windowHeight));
  }

  void MoveShip()
  {
    glm::vec3 wscp = GetWSCP() + glm::vec3(0, 0, 5);
    glm::vec3 moveVec = (wscp - m_spaceShip->m_node->m_translation) / 20.0f;
    m_spaceShip->m_node->m_translation += (glm::vec3(moveVec.x, 0, moveVec.z));

    m_spaceShip->m_node->m_orientation = glm::angleAxis(glm::radians(moveVec.x * 50), ToolKit::Z_AXIS);
  }

  void CheckProjectileMeteorCollision()
  {
    for (int i = (int)m_meteorManager.m_meteors.size() - 1; i > -1; i--)
    {
      bool markForDel = false;
      for (int j = (int)m_projectileManager.m_projectiles.size() - 1; j > -1; j--)
      {
        if (SpherePointIntersection(
          m_meteorManager.m_meteors[i]->m_node->m_translation,
          m_meteorManager.m_meteors[i]->m_collisionRadius, 
          m_projectileManager.m_projectiles[j]->m_node->m_translation))
        {
          SafeDel(m_projectileManager.m_projectiles[j]);
          m_projectileManager.m_projectiles.erase(m_projectileManager.m_projectiles.begin() + j);
          markForDel = true;
        }
      }
      if (markForDel)
      {
        ToolKit::Node* meteorNode = m_meteorManager.m_meteors[i]->m_node;
        m_explotionManager.SpawnMeteorExplosion(GetSSP(meteorNode->GetTransform(TransformationSpace::TS_WORLD)));

        if (m_meteorManager.m_meteors[i]->m_speed > 0.3f)
          m_score += 30;
        else
          m_score++;

        SafeDel(m_meteorManager.m_meteors[i]);
        m_meteorManager.m_meteors.erase(m_meteorManager.m_meteors.begin() + i);
        ToolKit::AudioPlayer::Play(&m_explosionSource);
      }
    }
  }

  void MoveFogLayer()
  {
    m_paralaxLayer.m_node->m_translation = -(m_spaceShip->m_node->m_translation - m_paralaxLayer.m_node->m_translation) / 24.0f;
  }

  void SetScoreText()
  {
    static int oldScore = m_score;
    if (m_score != oldScore)
    {
      m_scoreTxt->SetText("Score : " + std::to_string(m_score));
      oldScore = m_score;
    }
  }

  void RenderGui()
  {
    m_renderer.Render2d(m_crosshair, glm::ivec2(m_windowWidth, m_windowHeight));
    m_renderer.Render2d(m_scoreTxt->m_surface, glm::ivec2(m_windowWidth, m_windowHeight));
  }

  void Render()
  {
    m_renderer.Render(&m_backGround, &m_cam);
    m_renderer.Render(&m_paralaxLayer, &m_cam);
    m_renderer.Render(m_spaceShip, &m_cam);
    
    for (auto entry : m_projectileManager.m_projectiles)
      m_renderer.Render(entry, &m_cam);

    for (auto entry : m_meteorManager.m_meteors)
      m_renderer.Render(entry, &m_cam);

    for (auto entry : m_powerUpManager.m_onGoingPowerUps)
      m_renderer.Render(entry, &m_cam);

    for (auto entry : m_explotionManager.m_sprites)
      m_renderer.Render2d(entry, glm::ivec2(m_windowWidth, m_windowHeight));
  }

  void GameOver(int deltaTime)
  {
    m_renderer.Render(&m_backGround, &m_cam);
    m_renderer.Render(&m_paralaxLayer, &m_cam);
    
    for (auto entry : m_meteorManager.m_meteors)
      m_renderer.Render(entry, &m_cam);

    for (auto entry : m_explotionManager.m_sprites)
      m_renderer.Render2d(entry, glm::ivec2(m_windowWidth, m_windowHeight));

    m_renderer.Render2d(m_gameOverTxt->m_surface, glm::ivec2(m_windowWidth, m_windowHeight));
    m_renderer.Render2d(m_quitReplay->m_surface, glm::ivec2(m_windowWidth, m_windowHeight));

    m_meteorManager.Update(m_score);
    m_explotionManager.Update(deltaTime / 1000.0f);
    m_crosshair->m_node->m_translation = glm::vec3(m_sscp.x, m_sscp.y, 0);

    if (m_restartSignaled)
    {
      m_shipGone = false;
      m_restartSignaled = false;
      ResetGame();
    }
  }

  void SpawnNewHorde()
  {
    if (m_meteorManager.m_meteors.size() < 10)
    {
      for (int i = 0; i < 10; i++)
        m_meteorManager.Spawn();

      if (glm::linearRand(1, 2) == 2)
      {
        int count = glm::linearRand(1, 4);
        for (int j = 0; j < count; j++)
          m_meteorManager.Spawn(true);
      }
    }
  }

  void SpeedyMeteorMeteorCollisionCheck()
  {
    std::vector<int> removeList;
    for (int i = (int)m_meteorManager.m_meteors.size() - 1; i > -1; i--)
    {
      for (int j = 0; j < i; j++)
      {
        Meteor* a = m_meteorManager.m_meteors[i];
        Meteor* b = m_meteorManager.m_meteors[j];
        if (SphereSphereIntersection(
          a->m_node->m_translation, a->m_collisionRadius,
          b->m_node->m_translation, b->m_collisionRadius
          ))
        {
          if (glm::abs(b->m_speed - 0.3f) < 0.01)
          {
            removeList.push_back(j);
          }
        }
      }
    }

    std::sort(removeList.begin(), removeList.end());
    std::reverse(removeList.begin(), removeList.end());
    for (int i = 0; i < (int)removeList.size(); i++)
    {
      Meteor* meteor = m_meteorManager.m_meteors[removeList[i]];

      if (meteor->m_node->m_translation.z >= -10.0f)
        if (meteor->m_node->m_translation.z <= 10.0f)
          ToolKit::AudioPlayer::Play(&m_explosionSource);

      m_meteorManager.m_meteors.erase(m_meteorManager.m_meteors.begin() + removeList[i]);
      m_explotionManager.SpawnMeteorExplosion(GetSSP(meteor->m_node->GetTransform(TransformationSpace::TS_WORLD)));
      SafeDel(meteor);
    }
  }

  void CheckPowerUps()
  {
    if (m_powerUpManager.m_collectedPowerUps.empty() && m_powerUpManager.m_onGoingPowerUps.empty())
    {
      if (glm::linearRand(1, 2) == 2)
        m_powerUpManager.Spawn<FireRate2X>();

      if (glm::linearRand(1, 3) == 2)
        m_powerUpManager.Spawn<ForPower>();
    }
  }

  void GameUpdate(int deltaTime)
  {
    MoveShip();
    MoveFogLayer();
    m_projectileManager.UpdateProjectiles();

    m_crosshair->m_node->m_translation = glm::vec3(m_sscp.x, m_sscp.y, 0);

    m_meteorManager.Update(m_score);
    m_explotionManager.Update(deltaTime / 1000.0f);
    m_powerUpManager.UpdateCollecteds(deltaTime / 1000.0f);
    m_powerUpManager.UpdateOnGoings(m_spaceShip);

    CheckPowerUps();
    SpawnNewHorde();
    SpeedyMeteorMeteorCollisionCheck();
    CheckProjectileMeteorCollision();
    CheckShipMeteorCollision();
    SetScoreText();
  }

  void Frame(int deltaTime)
  {
    if (!m_shipGone)
    {
      GameUpdate(deltaTime);
      Render();
    }
    else
    {
      GameOver(deltaTime);
    }

    RenderGui();
  }

  void CheckShipMeteorCollision()
  {
    for (auto meteor : m_meteorManager.m_meteors)
    {
      if (m_spaceShip->CheckShipSphereCollision(meteor->m_node->m_translation, meteor->m_collisionRadius))
      {
        m_shipGone = true;
        glm::ivec2 explosionPoint = GetSSP(m_spaceShip->m_node->GetTransform(TransformationSpace::TS_WORLD));
        m_explotionManager.SpawnShipExplosion(explosionPoint);
        ToolKit::AudioPlayer::Play(&m_shipExplosionSource);
      }
    }
  }

  void ResetGame()
  {
    for (auto entry : m_projectileManager.m_projectiles)
      SafeDel(entry);
    m_projectileManager.m_projectiles.clear();

    for (auto entry : m_meteorManager.m_meteors)
      SafeDel(entry);
    m_meteorManager.m_meteors.clear();

    for (auto entry : m_explotionManager.m_sprites)
      SafeDel(entry);
    m_explotionManager.m_sprites.clear();

    for (auto entry : m_powerUpManager.m_collectedPowerUps)
      SafeDel(entry);
    m_powerUpManager.m_collectedPowerUps.clear();

    for (auto entry : m_powerUpManager.m_onGoingPowerUps)
      SafeDel(entry);
    m_powerUpManager.m_onGoingPowerUps.clear();

    m_score = 0;
    m_spaceShip->m_node->m_translation = glm::vec3();
  }

  ToolKit::Camera m_cam;
  ToolKit::Renderer m_renderer;
  Ship* m_spaceShip = nullptr;
  ToolKit::Surface* m_crosshair = nullptr;
  glm::ivec2 m_sscp;
  bool m_shipGone = false;
  ProjectileManager m_projectileManager;
  MeteorManager m_meteorManager;
  ExplosionManager m_explotionManager;
  PowerUpManager m_powerUpManager;
  ToolKit::Drawable m_backGround;
  ToolKit::Drawable m_paralaxLayer;
  TTFText* m_scoreTxt;
  TTFText* m_gameOverTxt;
  TTFText* m_quitReplay;
  int m_score = 0;
  bool m_restartSignaled = false;
  std::shared_ptr<ToolKit::Audio> m_lazerShotWav;
  std::shared_ptr<ToolKit::Audio> m_explosionWav;
  std::shared_ptr<ToolKit::Audio> m_shipExplosionWav;
  std::shared_ptr<ToolKit::Audio> m_ambientLoopWav;
  ToolKit::AudioSource m_lazerShotSource;
  ToolKit::AudioSource m_explosionSource;
  ToolKit::AudioSource m_shipExplosionSource;
  ToolKit::AudioSource m_ambientLoopSource;

  int m_windowWidth;
  int m_windowHeight;
};

extern App* g_app;