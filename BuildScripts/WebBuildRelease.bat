mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DWEB_BUILD:INT=1 -DTK_CXX_EXTRA:STRING="-O3" -S ../../ -G Ninja && ninja & set exit_code = %errorlevel% & pause & exit exit_code