mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DTK_CXX_EXTRA:STRING="-O3" -S ../../ -G Ninja && ninja & pause