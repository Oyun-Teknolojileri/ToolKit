cmake -B "../Dependency/assimp/build" -S "../Dependency/assimp" -G "Visual Studio 17 2022"

cd "../Dependency/assimp/build"

cmake --build . --config Release

cd "bin/Release"

copy "assimp-vc143-mt.dll" "../../../../../Import/assimp-vc143-mt.dll"
copy "assimp-vc143-mt.pdb" "../../../../../Import/assimp-vc143-mt.pdb"

cd "../../lib/Release"

copy "assimp-vc143-mt.lib" "../../../../../Utils/Import/assimp-vc143-mt.lib"