cmake -B build -DCMAKE_BUILD_TYPE=Debug -S .. -G "Visual Studio 17 2022"
cmake --build build --config Debug & set exit_code = %errorlevel% & exit exit_code
