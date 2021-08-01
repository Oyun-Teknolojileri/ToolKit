#include "stdafx.h"
#include "ResourceManager.h"
#include "Util.h"
#include "Animation.h"
#include "Audio.h"
#include "Material.h"
#include "Mesh.h"
#include "Shader.h"
#include "SpriteSheet.h"
#include "Texture.h"
#include "Scene.h"
#include "DebugNew.h"

#include <string>

namespace ToolKit
{
  void ResourceManager::Init()
  {
    Logger::GetInstance()->Log("Initiating manager " + GetTypeString(m_type));
  }

  void ResourceManager::Uninit()
  {
    Logger::GetInstance()->Log("Uninitiating manager " + GetTypeString(m_type));
    m_storage.clear();
  }

  ResourceManager::~ResourceManager()
  {
    assert(m_storage.size() == 0); // Uninitialize all resources before exit.
  }

  bool ResourceManager::Exist(String file)
  {
    return m_storage.find(file) != m_storage.end();
  }
}
