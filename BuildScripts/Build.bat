@echo off
setlocal

:: Check the usage.
if "%~1"=="" goto :usage
if "%~2"=="" goto :usage
goto :continue

:usage
echo Usage: "Build.bat [Debug|Release|RelWithDebInfo|MinSizeRel] [Relative path to CMakeLists.txt]"
pause
exit /b 1

:continue

:: Set the visual studio compiler tool set for ninja.
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if %errorlevel% neq 0 (
    echo Failed to set up Visual Studio environment variables.
    pause
	exit /b 1
)

:: Set execution directory as the bat file' location.
CD /D "%~dp0"

:: Create directories for build artifacts.
mkdir ninjaBuild
cd ninjaBuild

:: Call the cmake to generate project files for ninja.
cmake -B build -DCMAKE_BUILD_TYPE=%~1 -DWIN_BUILD:INT=1 -S %~2 -G Ninja
if %errorlevel% neq 0 (
    echo Failed to generate Ninja build files.
    pause
	exit /b 1
)

:: Build with ninja.
cd build
ninja
if %errorlevel% neq 0 (
    echo Build failed.
    pause
    exit /b 1
)

endlocal
