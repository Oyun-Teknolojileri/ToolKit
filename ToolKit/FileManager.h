/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Types.h"

#include <zlib.h>

#ifdef TK_ANDROID
  #include <Android/minizip/unzip.h>
  #include <Android/minizip/zip.h>
#else
  #include <unzip.h>
  #include <zip.h>
#endif

namespace ToolKit
{

  /**
   * Maintain access to resources. This class can work either on Pak files, the resources zipped in "MinResources.pak"
   * or files that resides in Resources folder of the current project. Any access to resources should use FileManager in
   * the Main to keep its cross platform capabilities effective in all platforms.
   */
  class TK_API FileManager
  {
   public:
    FileManager();  //!< Empty constructor.
    ~FileManager(); //!< Closes pak file if any open.

    /** Returns an xml file. */
    XmlFilePtr GetXmlFile(const String& filePath);

    /** Returns an image raw data and its properties in the arguments. Used in Texture::Load to create resource. */
    uint8* GetImageFile(const String& filePath, int* x, int* y, int* comp, int reqComp);

    /** Returns a hdri image raw data and its properties in the arguments. Used in Texture::Load to create resource. */
    float* GetHdriFile(const String& filePath, int* x, int* y, int* comp, int reqComp);

    /** Returns a decoded audio file or null if no decoder found. Used in Audio::Load to create resource. */
    SoundBuffer GetAudioFile(const String& filePath);

    /**
     * Pack all the resources for the project.
     * Does this by opening all scene and layer files in resource folder.
     * Than accumulate all resources in all managers. Finally creates a zip file from the collected resources.
     * Produced zip file is called "MinResources.pak"
     * If extra files other than automatically collected ones are needed, the function looks for a text file
     * "ExtraFiles.txt" each line in this file is added to the pack as well.
     * All files must be in the Resources folder of the project.
     */
    int PackResources();

    bool CheckFileFromResources(const String& path); //!< Checks the given file in first resource path than in pak file.
    void GetRelativeResourcesPath(String& path);     //!< Converts the path, relative to the Resources folder.

    void CloseZipFile(); //!< Closes the pak file if its open.
    bool CheckPakFile(); //!< Returns true if workspace contains pak file.

    String ReadAllText(const String& file); //!< Read the text content of the file. Zip file not supported.
    void WriteAllText(const String& file, const String& text); //!< Write the text to the file. Zip file not supported.

   private:
    typedef std::variant<XmlFilePtr, uint8*, float*, SoundBuffer> FileDataType;

    enum class FileType
    {
      Xml,
      ImageUint8,
      ImageFloat,
      Audio
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
    void GetAllUsedResourcePaths();

    bool ZipPack(const String& zipName);
    bool AddFileToZip(zipFile zfile, const char* filename);

    void GetAllPaths(const String& path);
    void GetExtraFilePaths();

    XmlFilePtr ReadXmlFileFromZip(zipFile zfile, const String& relativePath, const char* path);
    uint8* ReadImageFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo);
    float* ReadHdriFileFromZip(zipFile zfile, const String& relativePath, ImageFileInfo& fileInfo);

    /** Reads the file from the zip and returns it as buffer pointer and set the buffer size. */
    ubyte* ReadFileBufferFromZip(zipFile zfile, const String& relativePath, uint& bufferSize);

    XmlFilePtr CreateXmlFileFromZip(zipFile zfile, const String& filename, uint filesize);
    uint8* CreateImageFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo);
    float* CreateHdriFileFromZip(zipFile zfile, uint filesize, ImageFileInfo& fileInfo);

    void GenerateOffsetTableForPakFiles();
    bool IsFileInPak(const String& filename);

   private:
    StringSet m_allPaths;
    std::unordered_map<String, std::pair<ZPOS64_T, uint>> m_zipFilesOffsetTable;
    bool m_offsetTableCreated = false;
    zipFile m_zfile           = nullptr;

   public:
    bool m_ignorePakFile = false;
  };
} // namespace ToolKit
