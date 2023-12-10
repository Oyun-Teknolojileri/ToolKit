:: build zlib as static library

cmake -B "../Dependency/zlib-1.2.12/build" -DCMAKE_BUILD_TYPE=Release -S "../Dependency/zlib-1.2.12" -G "Visual Studio 17 2022"

cd "../Dependency/zlib-1.2.12/build"

cmake --build . --config Release

if not exist "../dist/lib" (
	mkdir "../dist/lib"
)
cd Release
copy "zlibstatic.lib" "../../dist/lib/zlibstatic.lib"

:: build minizip

cd ../../contrib/minizip
cmake -B "build" -DCMAKE_BUILD_TYPE=Release -S "." -G "Visual Studio 17 2022"

cd build
cmake --build . --config Release

cd Release
copy "minizip.lib" "../../../../dist/lib/minizip.lib"

:: build minizip with emscripten
cd ../../
if not exist "webbuild" (
	mkdir "webbuild"
)
cd webbuild
emcmake cmake -DEMSCRIPTEN=TRUE -DTK_CXX_EXTRA:STRING="-O3" -S .. -G Ninja && ninja & copy "libminizip.a" "../../../dist/libminizip.a"