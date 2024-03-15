mkdir ninjaBuild & cd ninjaBuild & cmake -B build -DCMAKE_BUILD_TYPE=Debug -DWIN_BUILD:INT=1 -S ../../ -G Ninja && cd build && ninja && set exit_code=%errorlevel% && exit /b %exit_code%
