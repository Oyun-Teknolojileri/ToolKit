#pragma once

#include <filesystem>

#include "Types.h"

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    void PackResources(const String& path);

   private:
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths();
    void CreatePackResources(const String& path);
    void CreatePackDirectories();

    void CopyFontResourcesToPack();
    void CopyMaterialResourcesToPack();
    void CopyMeshResourcesToPack();
    void CopyShaderResourcesToPack();
    void CopyTextureResourcesToPack();
    void CopyAnimationResourcesToPack(const String& path);
    void CopySceneResourcesToPack(const String& path);

   private:
    UniqueStringArray m_fontResourcePaths;
    UniqueStringArray m_materialResourcePaths;
    UniqueStringArray m_meshResourcePaths;
    UniqueStringArray m_shaderResourcePaths;
    UniqueStringArray m_textureResourcePaths;

    Path m_minFontsDirectoryPath;
    Path m_minMaterialsDirectoryPath;
    Path m_minMeshesDirectoryPath;
    Path m_minShadersDirectoryPath;
    Path m_minTexturesDirectoryPath;
    Path m_minAnimDirectoryPath;
    Path m_minSceneDirectoryPath;
  };
}  // namespace ToolKit
