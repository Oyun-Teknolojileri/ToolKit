#include "stdafx.h"
#include "ToolKit.h"
#include "Logger.h"

std::string ToolKit::TexturePath(std::string file)
{
  std::string path = "../Resources/Textures/";
  path += file;
  return path;
}

std::string ToolKit::MeshPath(std::string file)
{
  std::string path = "../Resources/Meshes/";
  path += file;
  return path;
}

std::string ToolKit::FontPath(std::string file)
{
  std::string path = "../Resources/Fonts/";
  path += file;
  return path;
}

std::string ToolKit::SpritePath(std::string file)
{
  std::string path = "../Resources/Sprites/";
  path += file;
  return path;
}

std::string ToolKit::AudioPath(std::string file)
{
  std::string path = "../Resources/Audio/";
  path += file;
  return path;
}

std::string ToolKit::AnimationPath(std::string file)
{
  std::string path = "../Resources/Meshes/";
  path += file;
  return path;
}

std::string ToolKit::SkeletonPath(std::string file)
{
  std::string path = "../Resources/Meshes/";
  path += file;
  return path;
}

std::string ToolKit::ShaderPath(std::string file)
{
  std::string path = "../Resources/Shaders/";
  path += file;
  return path;
}

std::string ToolKit::MaterialPath(std::string file)
{
  std::string path = "../Resources/Materials/";
  path += file;
  return path;
}

ToolKit::Main ToolKit::Main::m_instance;

ToolKit::Main::~Main()
{
}

void ToolKit::Main::Init()
{
  Logger::GetInstance()->Log("ToolKit Initialization");
  m_animationMan.Init();
  m_textureMan.Init();
  m_meshMan.Init();
  m_skinMeshMan.Init();
  m_spriteSheetMan.Init();
  m_audioMan.Init();
  m_shaderMan.Init();
  m_materialManager.Init();
  
  m_initiated = true;
}

void ToolKit::Main::Uninit()
{
  m_animationPlayer.m_records.clear();
  m_animationMan.Uninit();
  m_textureMan.Uninit();
  m_meshMan.Uninit();
  m_skinMeshMan.Uninit();
  m_spriteSheetMan.Uninit();
  m_audioMan.Uninit();
  m_shaderMan.Uninit();
  m_materialManager.Uninit();

  m_initiated = false;
}

ToolKit::Main::Main()
{
}

ToolKit::Main* ToolKit::Main::GetInstance()
{
  return &m_instance;
}
