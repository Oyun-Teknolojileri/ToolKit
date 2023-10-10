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

#include "FileManager.h"

#include <Animation.h>
#include <Material.h>
#include <RenderSystem.h>
#include <TKOpenGL.h>
#include <Texture.h>
#include <ToolKit.h>
#include <Util.h>
#include <TKImage.h>
#include "Common/Win32Utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <chrono>
#include <mutex>
#include <thread>

#include "DebugNew.h"

#include <SDL.h>

namespace ToolKit
{
  static String activeProjectName;
  static String workspacePath    ;

  std::vector<String> messages;

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

    void AndroidPrepareIcon();
    void EditAndroidManifest();
    void AndroidRunOnPhone();
    int PackResources();

   public:
    String m_icon {};
    String m_appName {};
    int m_minSdk            = 27;
    int m_maxSdk            = 32;
    bool m_deployAfterBuild = false;
    bool m_isDebugBuild     = false;

    Oriantation m_oriantation;
    PublishPlatform m_platform = PublishPlatform::Android;
  };

  int Packer::PackResources()
  {
    String projectName = activeProjectName;
    if (projectName.empty())
    {
      GetLogger()->WriteConsole(LogType::Error, "No project is loaded!");
      return 0;
    }

    String sceneResourcesPath = ConcatPaths({ResourcePath(), "Scenes"});

    return GetFileManager()->PackResources(sceneResourcesPath);
  }

  int Packer::Publish()
  {
    if (!PackResources())
    {
      return 1;
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
      TK_ERR("unknown publish platform: %i\n", (int) m_platform);
      return 0;
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
      return 1;
    }
    std::filesystem::create_directories(publishBinDir, ec);
    if (returnLoggingError(publishBinDir))
    {
      return 1;
    }
    std::filesystem::create_directories(publishConfigDir, ec);
    if (returnLoggingError(publishConfigDir))
    {
      return 1;
    }

    TK_LOG("Run toolkit compile script\n");
    Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
    std::filesystem::current_path(newWorkDir);
    int toolKitCompileResult = FileManager::RunPipe("WinBuildRelease.bat", nullptr);
    if (toolKitCompileResult != 0)
    {
      returnLoggingError("WinBuildRelease", true);
      TK_ERR("ToolKit could not be compiled\n");
      return 1;
    }

    // Run plugin compile script
    newWorkDir = Path(ConcatPaths({ResourcePath(), "..", "Windows"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string()))
    {
      return 1;
    }

    int pluginCompileResult = FileManager::RunPipe("WinBuildRelease.bat", nullptr);
    if (pluginCompileResult != 0)
    {
      returnLoggingError("WinBuildRelease.bat", true);
      TK_ERR("Windows build has failed!\n");
      return 1;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string()))
    {
      return 1;
    }

    std::filesystem::create_directories(ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}));

    const String exeFile =
        ConcatPaths({ResourcePath(), "..", "Codes", "Bin"}) + GetPathSeparatorAsStr() + projectName + ".exe";

    const String pakFile                = ConcatPaths({ResourcePath(), "..", "MinResources.pak"});
    const String sdlDllPath             = ConcatPaths({workDir.string(), "SDL2.dll"});
    const String configDirectory        = ConcatPaths({ResourcePath(), "..", "Config"});
    const String engineSettingsPath     = ConcatPaths({ConfigPath(), "Engine.settings"});
    const String destEngineSettingsPath = ConcatPaths({publishConfigDir, "Engine.settings"});

    TK_LOG("Windows build done, moving files\n");

    // Copy exe file
    std::filesystem::copy(exeFile, publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishBinDir))
    {
      return 1;
    }

    // Copy SDL2.dll from ToolKit bin folder to publish bin folder
    std::filesystem::copy(sdlDllPath, publishBinDir, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishBinDir))
    {
      return 1;
    }

    // Copy pak
    std::filesystem::copy(pakFile, publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
    if (returnLoggingError(publishDirectory))
    {
      return 1;
    }

    // Copy engine settings to config folder
    std::filesystem::copy(engineSettingsPath,
                          destEngineSettingsPath,
                          std::filesystem::copy_options::overwrite_existing,
                          ec);
    if (returnLoggingError(engineSettingsPath))
    {
      return 1;
    }

    // Tell user about where the location of output files is
    GetLogger()->WriteConsole(LogType::Success, "Building for WINDOWS has been completed successfully.\n");
    GetLogger()->WriteConsole(LogType::Memo,
                              "Output files location: %s\n",
                              std::filesystem::absolute(publishDirectory).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirectory);
    return 0;
  }

  void Packer::AndroidPrepareIcon()
  {
    if (m_icon.empty() || m_icon == "default")
    {
      return;
    }
    String assetsPath = NormalizePath("Android/app/src/main/res");
    String projectName = activeProjectName;
    String resLocation = ConcatPaths({workspacePath, projectName, assetsPath});
    
    TK_LOG("Preparing Icons");
    int refWidth, refHeight, refComp;
    unsigned char* refImage = ImageLoad(m_icon.c_str(), &refWidth, &refHeight, &refComp, 0);

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

  void Packer::AndroidRunOnPhone()
  {
    // adb path is in: 'android-sdk/platform-tools'
    String sdkPath = String(getenv("ANDROID_HOME"));
    if (sdkPath.empty())
    {
      TK_WRN("ANDROID_HOME environment variable is not set.");
      return;
    }
    Path oldPath = std::filesystem::current_path();
    std::filesystem::current_path(ConcatPaths({sdkPath, "platform-tools"}));
    TK_LOG("Trying to execute the app on your phone...");

    const auto checkIfFailedFn = [oldPath](int execResult, const String& command) -> bool
    {
      if (execResult == 1)
      {
        TK_LOG("%s command failed! exec result: %i", command.c_str(), execResult);
        TK_WRN("Make sure that an android device is connected to your PC");
        TK_WRN("if still doesn't work uninstall application and rebuild.");
        std::filesystem::current_path(oldPath);
        return true;
      }
      return false;
    };

    String apkPath     = NormalizePath("Android\\app\\build\\outputs\\apk");
    apkPath            = ConcatPaths({apkPath, m_isDebugBuild ? "debug" : "release"});
    apkPath            = ConcatPaths({apkPath, m_isDebugBuild ? "app-debug.apk" : "app-release-unsigned.apk"});

    String projectName = activeProjectName;
    String apkLocation = ConcatPaths({workspacePath, projectName, apkPath});
    String packageName = "com.otyazilim.toolkit/com.otyazilim.toolkit.MainActivity"; // adb uses / forward slash

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

  void Packer::EditAndroidManifest()
  {
    TK_LOG("Editing Android Manifest\n");
    String projectName     = activeProjectName;
    String applicationName = m_appName;
    String mainPath        = NormalizePath("Android/app/src/main");
    // get manifest file from template
    String androidManifest = GetFileManager()->ReadAllText(
        std::filesystem::absolute(ConcatPaths({"..", "Template", mainPath, "AndroidManifest.xml"})).string());
    // replace template values with our settings
    ReplaceFirstStringInPlace(androidManifest, "@string/app_name", applicationName);
    ReplaceFirstStringInPlace(androidManifest,
                              "minSdkVersion=\"26\"",
                              "minSdkVersion=\"" + std::to_string(m_minSdk) + "\"");
    ReplaceFirstStringInPlace(androidManifest,
                              "maxSdkVersion=\"33\"",
                              "maxSdkVersion=\"" + std::to_string(m_maxSdk) + "\"");
    // 0 undefined 1 landscape 2 Portrait
    String oriantationNames[3] {"fullSensor", "landscape", "portrait"};
    ReplaceFirstStringInPlace(androidManifest,
                              "screenOrientation=\"portrait\"",
                              "screenOrientation=\"" + oriantationNames[(int) m_oriantation] + "\"");

    String mainLocation = ConcatPaths({workspacePath, projectName, mainPath});
    String manifestLoc  = ConcatPaths({mainLocation, "AndroidManifest.xml"});

    GetFileManager()->WriteAllText(manifestLoc, androidManifest);
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
      GetLogger()->WriteConsole(LogType::Error, "No project is loaded!\n");
      return 1;
    }

    String assetsPath = NormalizePath("Android/app/src/main/assets");
    String projectLocation                         = ConcatPaths({workspacePath, projectName});
    String sceneResourcesPath                      = ConcatPaths({projectLocation, "MinResources.pak"});
    String androidResourcesPath                    = ConcatPaths({projectLocation, assetsPath, "MinResources.pak"});

    const std::filesystem::copy_options copyOption = std::filesystem::copy_options::overwrite_existing;

    std::filesystem::copy(sceneResourcesPath, androidResourcesPath, copyOption, ec);
    if (returnLoggingError())
    {
      return 1;
    }

    EditAndroidManifest();

    std::filesystem::current_path(ConcatPaths({projectLocation, "Android"}), ec);
    if (returnLoggingError())
    {
      return 1;
    }

    AndroidPrepareIcon();

    TK_LOG("Building android apk, Gradle scripts running...\n");

    // use "gradlew bundle" command to build .aab project or use "gradlew assemble" to release build
    String command    = m_isDebugBuild ? "gradlew assembleDebug" : "gradlew assemble";
    
    int compileResult = FileManager::RunPipe(command, nullptr);
    {
      if (compileResult == 1)
      {
        GetLogger()->WriteConsole(LogType::Error, "Android build failed.\n");
        return 1;
      }

      String buildLocation = NormalizePath(ConcatPaths({projectLocation, "Android/app/build/outputs/apk"}));
      buildLocation        = ConcatPaths({buildLocation, m_isDebugBuild ? "debug" : "release"});
      const String publishDirStr   = ConcatPaths({ResourcePath(), "..", "Publish", "Android"});
      const String apkName         = m_isDebugBuild ? "app-debug.apk" : "app-release-unsigned.apk";
      const String apkPathStr      = ConcatPaths({buildLocation, apkName});

      projectName                  = m_appName;
      projectName                 += m_isDebugBuild ? "_debug.apk" : "_release.apk";
      const String publishApkPath  = ConcatPaths({publishDirStr, projectName});

      // Create directories
      std::filesystem::create_directories(publishDirStr, ec);
      if (returnLoggingError())
      {
        return 1;
      }

      std::filesystem::copy(apkPathStr, publishApkPath, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError())
      {
        return 1;
      }

      // Tell user about where the location of output files is
      GetLogger()->WriteConsole(LogType::Success, "Building for ANDROID has been completed successfully.\n");
      GetLogger()->WriteConsole(LogType::Memo,
                                "Output files location: %s\n",
                                std::filesystem::absolute(publishDirStr).string().c_str());

      PlatformHelpers::OpenExplorer(publishDirStr);
    }

    if (compileResult != 0)
    {
      returnLoggingError("Compiling Fail", true);
      TK_ERR("Compiling failed.\n");
      return 1;
    }

    std::filesystem::current_path(workDir, ec); // set work directory back

    if (m_deployAfterBuild)
    {
      AndroidRunOnPhone();
    }

    if (returnLoggingError())
    {
      return 1;
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

    TK_LOG("Run toolkit compile script");
    Path newWorkDir(ConcatPaths({"..", "BuildScripts"}));
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string(), false, __LINE__))
    {
      return 1;
    }

    int toolKitCompileResult = FileManager::RunPipe("WebBuildRelease.bat", nullptr);
    if (toolKitCompileResult != 0)
    {
      returnLoggingError("bat failed", true, __LINE__);
      TK_ERR("ToolKit could not be compiled!\n");
      return 1;
    }
    newWorkDir                               = Path(ConcatPaths({ResourcePath(), "..", "Web"}));

    const String pluginWebBuildScriptsFolder = ConcatPaths({ResourcePath(), "..", "Web", "WebBuildRelease.bat"});

    // Run scripts
    std::filesystem::current_path(newWorkDir, ec);
    if (returnLoggingError(newWorkDir.string(), false, __LINE__))
    {
      return 1;
    }

    TK_LOG("Plugin web build\n");
    int pluginCompileResult = FileManager::RunPipe(pluginWebBuildScriptsFolder, nullptr);
    
    if (pluginCompileResult != 0)
    {
      returnLoggingError(pluginWebBuildScriptsFolder, true, __LINE__);
      TK_ERR("Web build has failed!\n");
      return 1;
    }
    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string(), false, __LINE__))
    {
      return 1;
    }

    // Move files to a directory
    String projectName           = activeProjectName;
    String publishDirectory      = ConcatPaths({ResourcePath(), "..", "Publish", "Web"});
    String firstPart             = ConcatPaths({ResourcePath(), "..", "Codes", "Bin", projectName}) + ".";
    String files[] = {firstPart + "data", firstPart + "html", firstPart + "js", firstPart + "wasm"};
    
    std::filesystem::create_directories(publishDirectory, ec);
    if (returnLoggingError(publishDirectory, false, __LINE__))
    {
      return 1;
    }

    for (int i = 0; i < ArraySize(files); i++)
    {
      std::filesystem::copy(files[i].c_str(), publishDirectory, std::filesystem::copy_options::overwrite_existing, ec);
      if (returnLoggingError(files[i], false, __LINE__))
      {
        return 1;
      }
    }

    // Create run script
    std::ofstream runBatchFile(ConcatPaths({publishDirectory, "Run.bat"}).c_str());
    runBatchFile << "emrun ./" + projectName + ".html";
    runBatchFile.close();

    std::filesystem::current_path(workDir, ec);
    if (returnLoggingError(workDir.string(), false, __LINE__))
    {
      return 1;
    }

    // Output user about where are the output files
    GetLogger()->WriteConsole(LogType::Success, "Building for web has been completed successfully.\n");
    GetLogger()->WriteConsole(LogType::Memo,
                              "Output files location: %s\n",
                              std::filesystem::absolute(publishDirectory).string().c_str());

    PlatformHelpers::OpenExplorer(publishDirectory);
    return 0;
  }

  int ToolKitMain(int argc, char* argv[])
  {
    // https://stackoverflow.com/questions/59828628/read-on-a-pipe-blocks-until-program-running-at-end-of-pipe-terminates-windows
    // enables writing to toolkit asynchronusly. life saver posix code: 
    setvbuf(stdout, NULL, _IONBF, 0); // Disable buffering on stdout. 
    PlatformHelpers::HideConsoleWindow();

    // Initialize ToolKit to serialize resources
    Main* g_proxy = new Main();
    Main::SetProxy(g_proxy);

    String publishArguments = GetFileManager()->ReadAllText("PublishArguments.txt");
    StringArray arguments;
    
    const auto whitePredFn = [](char c) { return c != '\n' && std::isspace(c); };
    // remove whitespace from string 
    erase_if(publishArguments, whitePredFn);

    Split(publishArguments, "\n", arguments);
    Packer packer {};
    activeProjectName         = arguments[0];
    workspacePath             = arguments[1];
    packer.m_appName          = arguments[2];
    packer.m_deployAfterBuild = std::atoi(arguments[3].c_str());
    packer.m_isDebugBuild     = std::atoi(arguments[4].c_str());
    packer.m_minSdk           = std::atoi(arguments[5].c_str());
    packer.m_maxSdk           = std::atoi(arguments[6].c_str());
    packer.m_oriantation      = (Oriantation)std::atoi(arguments[7].c_str());
    packer.m_platform         = (PublishPlatform)std::atoi(arguments[8].c_str());
    packer.m_icon             = arguments[9];

    // Set resource root to project's Resources folder
    g_proxy->m_resourceRoot = ConcatPaths({workspacePath, activeProjectName, "Resources"});

    g_proxy->SetConfigPath(ConcatPaths({"..", "Config"}));
    g_proxy->PreInit();

    GetLogger()->SetWriteConsoleFn([](LogType lt, String ms) -> void
    {
      printf("%s", ms.c_str());
    });

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

    getchar();
    return result;
  }
} // namespace ToolKit

int main(int argc, char* argv[]) { return ToolKit::ToolKitMain(argc, argv); }