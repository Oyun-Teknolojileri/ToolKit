mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DTK_CXX_EXTRA:STRING="-O0 -g" -S ../../ -G Ninja && ninja & pause