:: build zlib as static library
cmake -B "../Dependency/minizip-ng/build" -DCMAKE_BUILD_TYPE=Release -S "../Dependency/minizip-ng" -G "Visual Studio 17 2022" -D MZ_COMPAT=ON -D ZSTD_BUILD_STATIC=ON -D ZSTD_BUILD_SHARED=OFF -D MZ_ZLIB=OFF  -D MZ_BZIP2=OFF -D MZ_LZMA=OFF -D MZ_FETCH_LIBS=ON -D MZ_FORCE_FETCH_LIBS=ON -D MZ_PKCRYPT=OFF -D MZ_WZAES=OFF -D MZ_ICONV=OFF

cd "../Dependency/minizip-ng/build"

cmake --build . --config Release

if not exist "../dist/lib" (
	mkdir "../dist/lib"
	mkdir "../dist/include"
)

:: copy compability files to dist/include
copy "zip.h" "../dist/include/zip.h"
copy "unzip.h" "../dist/include/unzip.h"
copy "ioapi.h" "../dist/include/ioapi.h"

:: copy libraries to dist/library
copy "Release\minizip.lib" "../dist/lib/minizip.lib"
copy "_deps\zstd-build\lib\Release\zstd_static.lib" "../dist/lib/zstd_static.lib"