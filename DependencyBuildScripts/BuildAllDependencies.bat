
set currentPath=%CD%

cd "%currentPath%"
call BuildSDL2.bat

cd "%currentPath%"
call BuildImGui.bat

:: build zlip last because emscripten pauses and exits the program when building with emcmake

cd "%currentPath%"
call BuildZLib.bat