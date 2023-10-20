cmake -B build -DCMAKE_BUILD_TYPE=Release -S .. -G "Visual Studio 17 2022"
cmake --build build --config Release & set exit_code = %errorlevel% & exit exit_code
