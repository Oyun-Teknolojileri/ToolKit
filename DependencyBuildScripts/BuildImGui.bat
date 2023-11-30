cmake -B "../Dependency/ImGui/build" -DCMAKE_BUILD_TYPE=Release -S "../Dependency/ImGui" -G "Visual Studio 17 2022"

cd "../Dependency/ImGui/build"

if not exist "lib" (
	mkdir "lib"
)

cmake --build . --config Release
cmake --build . --config Debug