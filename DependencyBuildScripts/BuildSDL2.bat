
if not exist "../Dependency/SDL2/CMakeLists.txt" (
	echo Update submodules to get SDL2
	pause
	exit
)

cmake -B "../Dependency/SDL2/build" -DCMAKE_BUILD_TYPE=Release -S "../Dependency/SDL2" -G "Visual Studio 17 2022"

cd "../Dependency/SDL2/build"

if not exist "Release" (
	mkdir "Release"
)

cmake --build . --config Release
cd Release
copy "SDL2.dll" "../../../../Bin/SDL2.dll"

set arg=%1 
if "%arg" equ "NOPAUSE" (
	exit
)
pause