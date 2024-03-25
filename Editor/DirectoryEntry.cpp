/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include "DirectoryEntry.h"

#include <Animation.h>
#include <Audio.h>
#include <Material.h>
#include <Mesh.h>
#include <Scene.h>
#include <Shader.h>
#include <Texture.h>
#include <ToolKit.h>

namespace ToolKit
{
  namespace Editor
  {

    DirectoryEntry::DirectoryEntry() {}

    DirectoryEntry::DirectoryEntry(const String& fullPath)
    {
      DecomposePath(fullPath, &m_rootPath, &m_fileName, &m_ext);
    }

    String DirectoryEntry::GetFullPath() const { return ConcatPaths({m_rootPath, m_fileName + m_ext}); }

    ResourceManager* DirectoryEntry::GetManager() const
    {
      using GetterFunction = std::function<ResourceManager*()>;

      static std::unordered_map<String, GetterFunction> extToResource {
          {ANIM,     GetAnimationManager},
          {AUDIO,    GetAudioManager    },
          {MATERIAL, GetMaterialManager },
          {MESH,     GetMeshManager     },
          {SKINMESH, GetMeshManager     },
          {SHADER,   GetShaderManager   },
          {HDR,      GetTextureManager  },
          {SCENE,    GetSceneManager    }
      };

      auto resourceManager = extToResource.find(m_ext);
      if (resourceManager != extToResource.end())
      {
        return resourceManager->second(); // call get function
      }

      if (SupportedImageFormat(m_ext))
      {
        return GetTextureManager();
      }

      return nullptr;
    }

    RenderTargetPtr DirectoryEntry::GetThumbnail() const { return g_app->m_thumbnailManager.GetThumbnail(*this); }

  } // namespace Editor
} // namespace ToolKit