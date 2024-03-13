cmake -Bbuild -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWIN_BUILD:INT=1 -S ../Codes -G "Visual Studio 17 2022" 
cmake --build build --config RelWithDebInfo & set exit_code = %errorlevel% & exit exit_code