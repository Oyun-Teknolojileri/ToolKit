cd /d "%~dp0"
mkdir ninjaBuild & cd ninjaBuild & emcmake cmake -DCMAKE_BUILD_TYPE=Debug -DWEB_BUILD:BOOL=1 -DTK_CXX_EXTRA:STRING="-O0 -g -pthread" -S ../../ -G Ninja && ninja || (pause & exit /b 1)