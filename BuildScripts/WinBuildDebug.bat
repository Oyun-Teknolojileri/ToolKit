cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug -S .. -G "Visual Studio 17 2022" & pause
cmake --build build --config Debug & & set exit_code = %errorlevel% & pause & exit exit_code
