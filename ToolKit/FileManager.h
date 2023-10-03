/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "Types.h"

#include <zlib.h>

#include <variant>

#ifdef __ANDROID__
  #include <Android/minizip/unzip.h>
  #include <Android/minizip/zip.h>
#else
  #include <unzip.h>
  #include <zip.h>
#endif

namespace ToolKit
{
  class TK_API FileManager
  {
   public:
    ~FileManager();

    XmlFilePtr GetXmlFile(const String& filePath);
    uint8* GetImageFile(const String& filePath, int* x, int* y, int* comp, int reqComp);
    float* GetHdriFile(const String& filePath, int* x, int* y, int* comp, int reqComp);

    void PackResources(const String& path);
    bool CheckFileFromResources(const String& path);
    void GetRelativeResourcesPath(String& path);
    bool CheckPakFile(); //!< Returns true if workspace contains pak file.
    String ReadAllText(const String& file);
    void WriteAllText(const String& file, const String& text);

   private:
    typedef std::variant<XmlFilePtr, uint8*, float*> FileDataType;

    enum class FileType
    {
      Xml,
      ImageUint8,
      ImageFloat
    };

    struct ImageFileInfo
    {
      String& filePath;
      int* x;
      int* y;
      int* comp;
      int reqComp;
    };

    FileDataType GetFile(FileType fileType, ImageFileInfo& fileInfo);
    void LoadAllScenes(const String& path);
    void GetAllUsedResourcePaths(const String& path);

    bool ZipPack(const String& zipName);
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAllPaths(const String& path);
    void GetExtraFilePaths(const String& path);

    XmlFilePtr ReadXmlFileFromZip(zipFile zfile, const String& relativePath, const char* path);
    uint8* ReadImageFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo);
    float* ReadHdriFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo);
    XmlFilePtr CreateXmlFileFromZip(zipFile zfile, const String& filename, uint filesize);
    uint8* CreateImageFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo);
    float* CreateHdriFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo);

    void GenerateOffsetTableForPakFiles();
    bool IsFileInPak(const String& filename);

   private:
    StringSet m_allPaths;
    std::unordered_map<String, std::pair<ZPOS64_T, uint32_t>> m_zipFilesOffsetTable;
    bool m_offsetTableCreated = false;
    zipFile m_zfile           = nullptr;
  };
} // namespace ToolKit
