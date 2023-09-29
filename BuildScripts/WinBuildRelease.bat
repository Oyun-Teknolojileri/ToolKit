cmake -B build -DCMAKE_BUILD_TYPE=Release -S .. -G "Visual Studio 17 2022" & pause
cmake --build build --config Release & set exit_code = %errorlevel% & pause & exit exit_code
