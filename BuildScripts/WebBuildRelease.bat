mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DCMAKE_BUILD_TYPE=Release -DTK_CXX_EXTRA:STRING="-O3 -pthread" -S ../../ -G Ninja && ninja & set exit_code = %errorlevel% & exit exit_code