@echo off
setlocal

if "%~1"=="" (
    echo Usage: "Build.bat [Debug|Release|RelWithDebInfo|MinSizeRel]"
    pause
	exit /b 1
)

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if %errorlevel% neq 0 (
    echo Failed to set up Visual Studio environment variables.
    pause
	exit /b 1
)

mkdir ninjaBuild
cd ninjaBuild
cmake -B build -DCMAKE_BUILD_TYPE=%~1 -DWIN_BUILD:INT=1 -S ../ -G Ninja
if %errorlevel% neq 0 (
    echo Failed to generate Ninja build files.
    pause
	exit /b 1
)

cd build
ninja
if %errorlevel% neq 0 (
    echo Build failed.
    pause
    exit /b 1
)

endlocal
