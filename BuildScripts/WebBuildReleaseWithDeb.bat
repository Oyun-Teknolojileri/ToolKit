mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DWEB_BUILD:INT=1 -DCMAKE_BUILD_TYPE=Release -DTK_CXX_EXTRA:STRING="-O3 -g -pthread" -S ../../ -G Ninja && ninja & set exit_code = %errorlevel% & pause & exit exit_code