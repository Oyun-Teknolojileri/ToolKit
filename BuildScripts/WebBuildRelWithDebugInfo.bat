mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DWEB_BUILD:INT=1 -DCMAKE_BUILD_TYPE=RelWithDebInfo -DTK_CXX_EXTRA:STRING="-O3 -g -pthread" -S ../../ -G Ninja && ninja & set exit_code = %errorlevel% & exit exit_code