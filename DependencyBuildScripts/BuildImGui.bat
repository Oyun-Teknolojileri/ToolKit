cmake -B "../Dependency/tkimgui/build" -S "../Dependency/tkimgui" -G "Visual Studio 17 2022"

cd "../Dependency/tkimgui/build"

if not exist "lib" (
	mkdir "lib"
)

cmake --build . --config Release
cmake --build . --config Debug

cd "../lib"
copy "imgui.dll" "../../../Bin/imgui.dll"
copy "imgui_d.dll" "../../../Bin/imgui_d.dll"