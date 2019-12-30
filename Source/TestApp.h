#pragma once

#include "Camera.h"
#include "Renderer.h"
#include "Mesh.h"
#include "Drawable.h"
#include "Cube.h"
#include "Texture.h"
#include "Player.h"
#include "ToolKit.h"
#include "AnimBox.h"
#include "SDL.h"

class TestApp
{
public:
  void Init()
  {
    m_renderer.InitBasicShaders();

    m_cam.Translate(glm::vec3(1.0f, 4.5f, 2.0f));
    m_cam.Pitch(glm::radians(-90.0f));
    m_cam.Translate(glm::vec3(0.0f, 1.0f, -1.0f));

    int index = 0;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        m_cubes[index].m_texture = ToolKit::Main::m_textureMan.Create(ToolKit::TexturePath("crate.png"));
        m_cubes[index++].m_node->Translate(glm::vec3(1.0f * j, 0.0f, 1.0f * i));
      }
    }

    m_abox.Translate(glm::vec3(1.0f, 1.5f, 1.0f));

    m_player.m_taskList.push_back(Player::RotationTask::Vertical_Plus);
    m_player.m_taskList.push_back(Player::RotationTask::Horizontal_Minus);
    m_player.m_taskList.push_back(Player::RotationTask::Vertical_Minus);
    m_player.m_taskList.push_back(Player::RotationTask::Horizontal_Plus);
    
    std::reverse(m_player.m_taskList.begin(), m_player.m_taskList.end());
  }

  void Frame()
  {
    // Render board always
    for (int i = 0; i < 9; i++)
      m_renderer.Render(&m_cubes[i], &m_cam);

    // Only draw during anim sequance
    if (!m_abox.m_closeSeqFinished)
    {
      for (int i = 0; i < 6; i++)
        m_renderer.Render(m_abox.m_planes[i], &m_cam);
      m_abox.CloseAnim();
    }
    else
    {
      if (m_waitAbox)
      {
        m_waitAbox = false;
        m_startRolling = true;
        SDL_Delay(500);
      }
    }

    if (m_startRolling)
    {
      m_renderer.Render(&m_player, &m_cam);

      if (m_player.m_taskList.size())
      {
        switch (m_player.m_taskList.back())
        {
        case Player::Vertical_Plus:
          m_player.RotateVerticalPlus();
          m_player.m_prevTask = Player::Vertical_Plus;
          break;
        case Player::Vertical_Minus:
          m_player.RotateVerticalMinus();
          m_player.m_prevTask = Player::Vertical_Minus;
          break;
        case Player::Horizontal_Plus:
          m_player.RotateHorizontalPlus();
          m_player.m_prevTask = Player::Horizontal_Plus;
          break;
        case Player::Horizontal_Minus:
          m_player.RotateHorizontalMinus();
          m_player.m_prevTask = Player::Horizontal_Minus;
          break;
        }
      }
    }
  }

  ToolKit::Camera m_cam;
  ToolKit::Renderer m_renderer;
  ToolKit::Cube m_cubes[9];
  Player m_player;
  AnimBox m_abox;
  bool m_startRolling = false;
  bool m_waitAbox = true;
};

extern TestApp* g_app;