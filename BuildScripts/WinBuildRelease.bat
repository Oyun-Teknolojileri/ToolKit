cd /d "%~dp0"
mkdir ninjaBuild & cd ninjaBuild & cmake -B build -DCMAKE_BUILD_TYPE=Release -DWIN_BUILD:INT=1 -S ../../ -G Ninja && cd build && ninja || (pause & exit /b 1)
