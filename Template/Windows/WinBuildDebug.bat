cmake -DCMAKE_BUILD_TYPE=Debug -DWIN_BUILD:INT=1 -S .. -G "Visual Studio 17 2022" & pause
cmake --build . --config Debug & pause