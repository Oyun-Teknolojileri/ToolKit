cmake -B "../Dependency/assimp/build" -S "../Dependency/assimp" -G "Visual Studio 17 2022"

cd "../Dependency/assimp/build"

cmake --build . --config Release

cd "bin/Release"

copy "assimp-vc143-mt.dll" "../../../../../Utils/Import/assimp-vc143-mt.dll"
copy "assimp-vc143-mt.pdb" "../../../../../Utils/Import/assimp-vc143-mt.pdb"

cd "../../lib/Release"

copy "assimp-vc143-mt.lib" "../../../../../Import/assimp-vc143-mt.lib"

cd "../../"

cmake --build . --config Debug

cd "bin/Debug"

copy "assimp-vc143-mtd.dll" "../../../../../Utils/Import/assimp-vc143-mtd.dll"
copy "assimp-vc143-mtd.pdb" "../../../../../Utils/Import/assimp-vc143-mtd.pdb"

cd "../../lib/Debug"

copy "assimp-vc143-mtd.lib" "../../../../../Import/assimp-vc143-mtd.lib"