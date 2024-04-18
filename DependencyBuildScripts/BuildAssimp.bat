cmake -B "../Dependency/assimp/build" -S "../Dependency/assimp" -G "Visual Studio 17 2022"

cd "../Dependency/assimp/build"

cmake --build . --config Release

cd "bin/Release"

copy "assimp-vc143-mt.dll" "../../../../../Utils/Import/assimp-vc143-mt.dll"
copy "assimp-vc143-mt.pdb" "../../../../../Utils/Import/assimp-vc143-mt.pdb"

cd "../../lib/Release"

copy "assimp-vc143-mt.lib" "../../../../../Import/assimp-vc143-mt.lib"