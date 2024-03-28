cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DWIN_BUILD:INT=1 -S ../Codes -G "Visual Studio 17 2022" 
cmake --build build --config Release || (pause & exit /b 1)