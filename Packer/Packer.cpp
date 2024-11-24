/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#include <Animation.h>
#include <Common/Win32Utils.h>
#include <FileManager.h>
#include <Material.h>
#include <RenderSystem.h>
#include <SDL.h>
#include <TKImage.h>
#include <TKOpenGL.h>
#include <Texture.h>
#include <ToolKit.h>
#include <Types.h>
#include <Util.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <mutex>
#include <thread>

namespace ToolKit
{
  // Same enum that is inside Editor::PublisManager
  enum class PublishConfig
  {
    Debug   = 0, // Debug build
    Develop = 1, // Release build
    Deploy  = 2  // Release build with calling packer
  };

  static String activeProjectName;
  static String workspacePath;

  StringArray messages;

  enum class PublishPlatform
  {
    Web,
    Windows,
    Linux,
    Android
  };

  enum Oriantation
  {
    Undefined,
    Landscape,
    Portrait
  };

  class Packer
  {
   public:
    int Publish();
    int WindowsPublish();
    int WebPublish();
    int AndroidPublish();

    void SetAndroidOptions();
    void AndroidPrepareIcon();
    void EditAndroidManifest();
    void SetAndroidSDKVersion();
    void SetAndroidGameName();
    void AndroidRunOnPhone(bool isAPKUnsigned);
    int PackResources();

   public:
    /** Just packs the resources. Does not perform publishing. */
    bool m_onlyPack = false;
    String m_icon;
    String m_appName;
    int m_minSdk            = 27;
    int m_maxSdk            = 32;
    bool m_deployAfterBuild = false;
    PublishConfig m_publishConfig;
    PublishPlatform m_platform = PublishPlatform::Android;
    String m_toolkitPath;
    String m_templateGameFolderPath;
    Oriantation m_oriantation;
  };

  int Packer::PackResources()
  {
    String projectName = activeProjectName;
    if (projectName.empty())
    {
      TK_ERR("No project is loaded.");
      return -1;
    }

    int packResult = GetFileManager()->PackResources();
    if (packResult != 0)
    {
      return packResult;
    }

    return 0;
  }

  int Packer::Publish()
  {
    String packPath   = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});

    bool needPacking  = m_publishConfig == PublishConfig::Deploy;
    needPacking      |= !std::filesystem::exists(packPath);
    needPacking      |= m_onlyPack;

    if (needPacking)
    {
      int packResult = PackResources();
      if (packResult != 0 || m_onlyPack)
      {
        return packResult;
      }
    }

    switch (m_platform)
    {
    case ToolKit::PublishPlatform::Web:
      return WebPublish();
      break;
    case ToolKit::PublishPlatform::Windows:
      return WindowsPublish();
      break;
    case ToolKit::PublishPlatform::Android:
      return AndroidPublish();
    default:
      TK_ERR("Unknown publish platform: %i\n", (int) m_platform);
      return -1;
    }
  }

  int Packer::WindowsPublish()
  {
    TK_LOG("Building for Windows\n");
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](String path = "", bool setPathBack = false) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("%s %s", ec.message().c_str(), path.c_str());
        setback = true;
        ret     = true;
      }

      if (setback)
      {
        std::filesystem::current_path(workDir, ec);
        if (ec)
        {
          TK_ERR("%s\n", ec.message().c_str(), path.c_str());
          TK_ERR("%s\n", "******** PLEASE RESTART THE EDITOR ********");
        }

        ret = true;
      }

      return ret;
    };

    // Move files to publish directory
    const String projectName      = activeProjectName;
    const String publishDirectory = ConcatPaths({ResourcePath(), "..", "Publish", "Windows"});
    const String publishBinDir    = ConcatPaths({publishDirectory, "Bin"});
    const String publishConfigDir = ConcatPaths({publishDirectory, "Config"});

    // Create directories
    std::filesystem::create_directories(publishDirectory, ec);
    if (returnLoggingError(publishDirectory))
    {
      return -1;
    }
    std::filesystem::create_directories(publishBinDir, ec);
    if (returnLoggingError(publishBinDir))
    {
      return -1;
    }
    std::filesystem::create_directories(publishConfigDir, ec);
    if (returnLoggingError(publishConfigDir))
    {
      return -1;
    }

    TK_LOG("Run toolkit compile script\n");
    Path newWorkDir(ConcatPaths({m_toolkitPath, "BuildScripts"}));
    std::filesystem::current_path(newWorkDir);

    int toolKitCompileResult = -1;
    String buildConfig;
    String script   = "Build.bat";
    String cmakeSrc = " ../../";

    if (m_publishConfig == PublishConfig::Debug)
    {
      buildConfig = " Debug";
    }
    else if (m_publishConfig == PublishConfig::Develop)
    {
      buildConfig = " RelWithDebInfo";
    }
    else
    {
      buildConfig = " Release";
    }

    String cmd           = script + buildConfig + cmakeSrc;
    toolKitCompileResult = std::system(cmd.c_str());
    if (toolKitCompileResult != 0)
    {
      returnLoggingError(cmd, true);
      TK_ERR("ToolKit could not be compiled\n");
      return -1;
    }

    // Run plugin compile script
    newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Windows"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string()))
    {
      return -1;
    }

    int pluginCompileResult = -1;
    cmakeSrc                = " ../../Codes";
    cmd                     = script + buildConfig + cmakeSrc;
    pluginCompileResult     = std::system(cmd.c_str());
    if (pluginCompileResult != 0)
    {
      returnLoggingError(cmd, true);
      TK_ERR("Windows build has failed!\n");
      return -1;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string()))
    {
      return -1;
    }

    std::filesystem::create_directories(ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}));

    const String exeFile =
        ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".exe";

    const String pakFile                = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
    const String sdlDllPath             = ConcatPaths({m_toolkitPath, "Bin", "SDL2.dll"});
    const String engineSettingsPath     = ConcatPaths({ResourcePath(), "..", "Config", "Windows", "Engine.settings"});
    const String destEngineSettingsPath = ConcatPaths({publishConfigDir, "Engine.settings"});

    TK_LOG("Windows build done, moving files\n");

    // Copy exe file
    std::filesystem::copy(exeFile, publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishBinDir))
    {
      return -1;
    }

    // Copy SDL2.dll from ToolKit bin folder to publish bin folder
    std::filesystem::copy(sdlDllPath, publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishBinDir))
    {
      return -1;
    }

    // Copy pak
    std::filesystem::copy(pakFile, publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishDirectory))
    {
      return -1;
    }

    // Copy engine settings to config folder
    std::filesystem::copy(engineSettingsPath,
                          destEngineSettingsPath,
                          std::filesystem::copy_options::overwrite_existing,
                          ec);
    if (returnLoggingError(engineSettingsPath))
    {
      return -1;
    }

    // Tell user about where the location of output files is
    TK_SUC("Building for WINDOWS has been completed successfully.\n");
    TK_LOG("Output files location: %s\n", std::filesystem::absolute(publishDirectory).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirectory);
    return 0;
  }

  void Packer::AndroidPrepareIcon()
  {
    String assetsPath  = NormalizePath("Android/app/src/main/res");
    String projectName = activeProjectName;
    String resLocation = ConcatPaths({workspacePath, projectName, assetsPath});

    TK_LOG("Preparing Icons\n");
    int refWidth, refHeight, refComp;
    unsigned char* refImage = ImageLoad(m_icon.c_str(), &refWidth, &refHeight, &refComp, 0);
    if (refImage == nullptr)
    {
      TK_LOG("Can not load icon image!\n");
      return;
    }

    // search each folder in res folder and find icons, replace that icons with new ones
    for (const auto& entry : std::filesystem::directory_iterator(resLocation))
    {
      if (!entry.is_directory())
      {
        continue;
      }

      for (auto& file : std::filesystem::directory_iterator(entry))
      {
        String name;
        String extension;
        String path = file.path().string();
        DecomposePath(path, nullptr, &name, &extension);
        // if this is image replace with new one, don't touch if this is background image
        if (name.find("background") == std::string::npos && extension == ".png")
        {
          int width, height, comp;
          // get the image that we want to replace
          unsigned char* img = ImageLoad(path.c_str(), &width, &height, &comp, 0);
          assert(img && "cannot load android icon");
          int res;
          res = ImageResize(refImage, refWidth, refHeight, 0, img, width, height, 0, comp);
          assert(res && "cannot resize android icon");
          // write resized image
          res = WritePNG(path.c_str(), width, height, comp, img, 0);
          assert(res && "cannot write to android icon");
          ImageFree(img);
        }
      }
    }
    ImageFree(refImage);
  }

  void Packer::AndroidRunOnPhone(bool isAPKUnsigned)
  {
    // adb path is in: 'android-sdk/platform-tools'
    String sdkPath = String(getenv("ANDROID_HOME"));
    if (sdkPath.empty())
    {
      TK_WRN("ANDROID_HOME environment variable is not set.\n ");
      return;
    }
    Path oldPath = std::filesystem::current_path();
    std::filesystem::current_path(ConcatPaths({sdkPath, "platform-tools"}));
    TK_LOG("Trying to execute the app on your phone...\n");

    const auto checkIfFailedFn = [oldPath](int execResult, const String& command) -> bool
    {
      if (execResult == 1)
      {
        TK_LOG("%s command failed! exec result: %i\n", command.c_str(), execResult);
        TK_WRN("Make sure that an android device is connected to your PC\n");
        TK_WRN("if still doesn't work uninstall application and rebuild.\n");
        std::filesystem::current_path(oldPath);
        return true;
      }
      return false;
    };

    String apkPath        = NormalizePath("Android/app/build/outputs/apk");

    String buildType      = m_publishConfig == PublishConfig::Debug ? "debug" : "release";
    apkPath               = ConcatPaths({apkPath, buildType});

    String releaseApkFile = isAPKUnsigned ? "app-release-unsigned.apk" : "app-release.apk";

    apkPath = ConcatPaths({apkPath, m_publishConfig == PublishConfig::Debug ? "app-debug.apk" : releaseApkFile});

    String projectName = activeProjectName;
    String apkLocation = ConcatPaths({workspacePath, projectName, apkPath});
    String packageName = "org.libsdl.app/org.libsdl.app.SDLActivity"; // adb uses forward slash

    int execResult;
    execResult = PlatformHelpers::SysComExec("adb install " + apkLocation, false, true, nullptr);
    if (checkIfFailedFn(execResult, "adb install " + apkLocation))
    {
      return;
    }

    execResult = PlatformHelpers::SysComExec("adb shell am start -n " + packageName, true, true, nullptr);
    if (checkIfFailedFn(execResult, "adb shell am start -n " + packageName))
    {
      return;
    }
    std::filesystem::current_path(oldPath);
  }

  void Packer::SetAndroidOptions()
  {
    SetAndroidGameName();
    SetAndroidSDKVersion();
    EditAndroidManifest();
  }

  void Packer::EditAndroidManifest()
  {
    TK_LOG("Editing Android Manifest\n");
    String projectName     = activeProjectName;
    String applicationName = m_appName;
    String mainPath        = NormalizePath("Android/app/src/main");

    // get manifest file from template
    String androidManifest = GetFileManager()->ReadAllText(
        std::filesystem::absolute(ConcatPaths({m_templateGameFolderPath, mainPath, "AndroidManifest.xml"})).string());

    // replace template values with our settings
    ReplaceFirstStringInPlace(androidManifest, "@string/app_name", applicationName);

    // 0 undefined 1 landscape 2 Portrait
    String oriantationNames[3] {"fullSensor", "landscape", "portrait"};
    ReplaceFirstStringInPlace(androidManifest,
                              "screenOrientation=\"landscape\"",
                              "screenOrientation=\"" + oriantationNames[(int) m_oriantation] + "\"");

    String mainLocation = ConcatPaths({workspacePath, projectName, mainPath});
    String manifestLoc  = ConcatPaths({mainLocation, "AndroidManifest.xml"});

    GetFileManager()->WriteAllText(manifestLoc, androidManifest);
  }

  void Packer::SetAndroidSDKVersion()
  {
    TK_LOG("Editing Build Gradle\n");
    String projectName     = activeProjectName;
    String applicationName = m_appName;
    String mainPath        = NormalizePath("Android/app");

    // get file from template
    String gradleFileText  = GetFileManager()->ReadAllText(
        std::filesystem::absolute(ConcatPaths({m_templateGameFolderPath, mainPath, "build.gradle"})).string());

    // replace template values with our settings
    ReplaceFirstStringInPlace(gradleFileText, "minSdkVersion 19", "minSdkVersion " + std::to_string(m_minSdk));
    ReplaceFirstStringInPlace(gradleFileText, "maxSdkVersion 34", "maxSdkVersion " + std::to_string(m_maxSdk));
    ReplaceFirstStringInPlace(gradleFileText, "compileSdkVersion 33", "compileSdkVersion " + std::to_string(m_maxSdk));

    String mainLocation = ConcatPaths({workspacePath, projectName, mainPath});
    String gradleLoc    = ConcatPaths({mainLocation, "build.gradle"});

    GetFileManager()->WriteAllText(gradleLoc, gradleFileText);
  }

  void Packer::SetAndroidGameName()
  {
    TK_LOG("Setting Game Name\n");
    const String mainPath =
        NormalizePath(ConcatPaths({"Android", "app", "src", "main", "res", "values", "strings.xml"}));

    // get file from template
    String stringsFileText = GetFileManager()->ReadAllText(
        std::filesystem::absolute(ConcatPaths({m_templateGameFolderPath, mainPath})).string());

    // replace template values with our settings
    ReplaceFirstStringInPlace(stringsFileText, "__GAME_NAME__", m_appName);

    String mainLocInProject = ConcatPaths({workspacePath, activeProjectName, mainPath});

    GetFileManager()->WriteAllText(mainLocInProject, stringsFileText);
  }

  int Packer::AndroidPublish()
  {
    TK_LOG("Building for android\n");
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](const String& path = " ", bool setPathBack = false) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("%s path: %s\n", ec.message().c_str(), path.c_str());
        setback = true;
        ret     = true;
      }

      if (setback)
      {
        std::filesystem::current_path(workDir, ec);
        if (ec)
        {
          TK_ERR("%s\n", ec.message().c_str());
          TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********");
        }

        ret = true;
      }

      return ret;
    };

    String projectName = activeProjectName;

    if (projectName.empty())
    {
      TK_ERR("No project is loaded.\n");
      return -1;
    }

    const String assetsPath             = NormalizePath("Android/app/src/main/assets");
    const String projectLocation        = ConcatPaths({workspacePath, projectName});
    const String sceneResourcesPath     = ConcatPaths({projectLocation, "MinResources.pak"});
    const String androidResourcesPath   = ConcatPaths({projectLocation, assetsPath, "MinResources.pak"});
    const String engineSettingsPath     = ConcatPaths({ResourcePath(), "..", "Config", "Android", "Engine.settings"});
    const String destEngineSettingsPath = ConcatPaths({projectLocation, assetsPath, "Config", "Engine.settings"});

    const std::filesystem::copy_options copyOption = std::filesystem::copy_options::overwrite_existing;

    std::filesystem::copy(sceneResourcesPath, androidResourcesPath, copyOption, ec);
    if (returnLoggingError("Copy: " + sceneResourcesPath + " and " + androidResourcesPath))
    {
      return -1;
    }

    std::filesystem::copy(engineSettingsPath, destEngineSettingsPath, copyOption, ec);
    if (returnLoggingError("Copy: " + engineSettingsPath + " and " + destEngineSettingsPath))
    {
      return -1;
    }

    SetAndroidOptions();

    String androidPath = ConcatPaths({projectLocation, "Android"});
    std::filesystem::current_path(androidPath, ec);
    if (returnLoggingError(androidPath))
    {
      return -1;
    }

    AndroidPrepareIcon();

    TK_LOG("Building android apk, Gradle scripts running...\n");

    String buildType     = m_publishConfig == PublishConfig::Debug ? "debug" : "release";

    // clean apk output directory
    String buildLocation = NormalizePath(ConcatPaths({projectLocation, "Android/app/build/outputs/apk"}));

    if (!std::filesystem::exists(buildLocation))
    {
      std::filesystem::create_directories(buildLocation);
    }

    for (auto& folder : std::filesystem::directory_iterator(buildLocation))
    {
      if (!folder.is_directory())
      {
        continue;
      }
      if (folder.path().filename().string() != buildType)
      {
        continue;
      }
      for (auto& file : std::filesystem::directory_iterator(folder))
      {
        if (file.is_directory())
        {
          continue;
        }
        std::filesystem::remove(file);
      }
    }

    // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
    String command    = m_publishConfig == PublishConfig::Debug ? "gradlew assembleDebug" : "gradlew assemble";

    int compileResult = std::system(command.c_str());
    if (compileResult != 0)
    {
      TK_ERR("Android build failed.\n");
      return -1;
    }

    buildLocation      = ConcatPaths({buildLocation, buildType});

    bool apkIsUnsigned = false;
    // See if the apk is unsigned or not
    if (std::filesystem::exists(ConcatPaths({buildLocation, "app-release-unsigned.apk"})))
    {
      apkIsUnsigned = true;
    }

    const String publishDirStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Android"});
    const String apkName         = m_publishConfig == PublishConfig::Debug ? "app-debug.apk"
                                   : apkIsUnsigned                         ? "app-release-unsigned.apk"
                                                                           : "app-release.apk";
    const String apkPathStr      = ConcatPaths({buildLocation, apkName});

    projectName                  = m_appName;
    projectName                 += m_publishConfig == PublishConfig::Debug ? "_debug.apk" : "_release.apk";
    const String publishApkPath  = ConcatPaths({publishDirStr, projectName});

    // Create directories
    std::filesystem::create_directories(publishDirStr, ec);
    if (returnLoggingError("Trying Create Dir:" + publishDirStr))
    {
      return -1;
    }

    std::filesystem::copy(apkPathStr, publishApkPath, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError("Trying Copy: " + apkPathStr + " and " + publishApkPath))
    {
      return -1;
    }

    // Tell user about where the location of output files is
    TK_SUC("Building for ANDROID has been completed successfully.\n");
    TK_LOG("Output files location: %s\n", std::filesystem::absolute(publishDirStr).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirStr);

    if (compileResult != 0)
    {
      returnLoggingError("Compiling Fail", true);
      TK_ERR("Compiling failed.\n");
      return -1;
    }

    std::filesystem::current_path(workDir, ec); // set work directory back
    if (returnLoggingError("Set Work Directory: " + workDir.string()))
    {
      return -1;
    }

    if (m_deployAfterBuild)
    {
      AndroidRunOnPhone(apkIsUnsigned);
    }

    return 0;
  }

  int Packer::WebPublish()
  {
    TK_LOG("Building for Web\n");
    Path workDir = std::filesystem::current_path();

    std::error_code ec;
    auto returnLoggingError = [&ec, &workDir](const String& path, bool setPathBack = false, int line = 0) -> bool
    {
      bool ret     = false;
      bool setback = setPathBack;
      if (ec)
      {
        TK_ERR("line %i; %s %s\n", line, ec.message().c_str(), path.c_str());
        setback = true;
        ret     = true;
      }

      if (setback)
      {
        std::filesystem::current_path(workDir, ec);
        if (ec)
        {
          TK_ERR("line %i; %s %s\n", line, ec.message().c_str(), path.c_str());
          TK_ERR("%s", "******** PLEASE RESTART THE EDITOR ********\n");
        }

        ret = true;
      }

      return ret;
    };

    String buildScriptName;
    if (m_publishConfig == PublishConfig::Debug)
    {
      buildScriptName = "WebBuildDebug.bat";
    }
    else if (m_publishConfig == PublishConfig::Develop)
    {
      buildScriptName = "WebBuildRelWithDebugInfo.bat";
    }
    else // if (m_publishConfig == PublishConfig::Deploy)
    {
      buildScriptName = "WebBuildRelease.bat";
    }

    // Compile ToolKit
    TK_LOG("Running Toolkit compile script");
    String tkBuildDir  = ConcatPaths({m_toolkitPath, "BuildScripts"});
    String tkBuildPath = ConcatPaths({tkBuildDir, buildScriptName});

    std::filesystem::current_path(tkBuildDir);
    int toolKitCompileResult = std::system(tkBuildPath.c_str());

    if (toolKitCompileResult != 0)
    {
      returnLoggingError(buildScriptName, true, __LINE__);
      TK_ERR("ToolKit could not be compiled!\n");
      return -1;
    }
    Path gameBuildDir          = Path(ConcatPaths({ResourcePath(), "..", "Web"}));
    const String gameBuildPath = ConcatPaths({ResourcePath(), "..", "Web", buildScriptName});

    // Compile game
    std::filesystem::current_path(gameBuildDir, ec);
    TK_LOG("Plugin web build\n");
    int pluginCompileResult = std::system(gameBuildPath.c_str());

    if (pluginCompileResult != 0)
    {
      returnLoggingError(gameBuildPath, true, __LINE__);
      TK_ERR("Web build has failed!\n");
      return -1;
    }

    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string(), false, __LINE__))
    {
      return -1;
    }

    // Move files to a directory
    String projectName      = activeProjectName;
    String publishDirectory = ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
    String firstPart        = ConcatPaths({ResourcePath(), "..", "Codes", "Bin", projectName}) + ".";
    String files[]          = {firstPart + "data",
                               firstPart + "html",
                               firstPart + "js",
                               firstPart + "wasm",
                               firstPart + "worker.js"};

    std::filesystem::create_directories(publishDirectory, ec);
    if (returnLoggingError(publishDirectory, false, __LINE__))
    {
      return -1;
    }

    for (int i = 0; i < ArraySize(files); i++)
    {
      std::filesystem::copy(files[i].c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError("Copy: " + files[i] + " to " + publishDirectory, false, __LINE__))
      {
        return -1;
      }
    }

    // Create run script
    std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
    runBatchFile << "emrun ./" + projectName + ".html";
    runBatchFile.close();

    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string(), false, __LINE__))
    {
      return -1;
    }

    // Output user about where are the output files
    TK_SUC("Building for web has been completed successfully.\n");
    TK_LOG("Output files location: %s\n", std::filesystem::absolute(publishDirectory).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirectory);
    return 0;
  }

  int ToolKitMain(int argc, char* argv[])
  {
    // Initialize ToolKit to serialize resources
    Main* g_proxy = new Main();
    Main::SetProxy(g_proxy);
    g_proxy->PreInit();

    String publishArguments = GetFileManager()->ReadAllText("PublishArguments.txt");
    StringArray arguments;

    const auto whitePredFn = [](char c) { return c != '\n' && std::isspace(c); };
    // remove whitespace from string
    erase_if(publishArguments, whitePredFn);

    Split(publishArguments, "\n", arguments);
    Packer packer {};
    activeProjectName         = NormalizePath(arguments[0]);
    workspacePath             = NormalizePath(arguments[1]);
    packer.m_appName          = NormalizePath(arguments[2]);
    packer.m_deployAfterBuild = std::atoi(arguments[3].c_str());
    packer.m_minSdk           = std::atoi(arguments[4].c_str());
    packer.m_maxSdk           = std::atoi(arguments[5].c_str());
    packer.m_oriantation      = (Oriantation) std::atoi(arguments[6].c_str());
    packer.m_platform         = (PublishPlatform) std::atoi(arguments[7].c_str());
    packer.m_icon             = arguments[8];
    packer.m_icon             = std::filesystem::absolute(packer.m_icon).string();
    packer.m_publishConfig    = (PublishConfig) std::atoi(arguments[9].c_str());
    packer.m_onlyPack         = std::atoi(arguments[10].c_str());

    // Set resource root to project's Resources folder
    g_proxy->m_resourceRoot   = ConcatPaths({workspacePath, activeProjectName, "Resources"});

    String toolkitAppdata     = ConcatPaths({getenv("APPDATA"), "ToolKit", "Config", "Path.txt"});
    String toolkitPath        = GetFileManager()->ReadAllText(toolkitAppdata);
    NormalizePathInplace(toolkitPath);
    packer.m_toolkitPath            = toolkitPath;
    packer.m_templateGameFolderPath = ConcatPaths({toolkitPath, "Templates", "Game"});
    g_proxy->SetConfigPath(ConcatPaths({toolkitPath, "Config"}));

    GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void { printf("%s", ms.c_str()); });

    // Init SDL
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_GAMECONTROLLER);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_Window* g_window    = SDL_CreateWindow("temp",
                                            SDL_WINDOWPOS_UNDEFINED,
                                            SDL_WINDOWPOS_UNDEFINED,
                                            32,
                                            32,
                                            SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    SDL_GLContext g_context = SDL_GL_CreateContext(g_window);

    g_proxy->m_renderSys->InitGl(SDL_GL_GetProcAddress, nullptr);

    g_proxy->Init();

    int result = packer.Publish();

    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(g_window);

    return result;
  }
} // namespace ToolKit

int main(int argc, char* argv[]) { return ToolKit::ToolKitMain(argc, argv); }