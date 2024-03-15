mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DCMAKE_BUILD_TYPE=Debug -DTK_CXX_EXTRA:STRING="-O0 -g -pthread" -S ../../ -G Ninja && ninja & pause & set exit_code = %errorlevel%  & exit exit_code