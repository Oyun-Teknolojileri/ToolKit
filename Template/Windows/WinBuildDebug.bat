cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug -DWIN_BUILD:INT=1 -S ../Codes -G "Visual Studio 17 2022" & pause
cmake --build build --config Debug & pause