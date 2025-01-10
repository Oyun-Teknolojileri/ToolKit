
set currentPath=%CD%

cd "%currentPath%"
call BuildSDL2.bat

cd "%currentPath%"
call BuildImGui.bat

cd "%currentPath%"
call BuildAssimp.bat

:: build minizip-ng last because emscripten pauses and exits the program when building with emcmake

cd "%currentPath%"
call BuildMinizipNg.bat
