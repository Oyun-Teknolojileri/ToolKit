cmake -DCMAKE_BUILD_TYPE=Release -DWIN_BUILD:INT=1 -S .. -G "Visual Studio 17 2022" & pause
cmake --build . --config Release & pause