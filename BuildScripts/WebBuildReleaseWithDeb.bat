mkdir ninjaBuild
cd ninjaBuild
emcmake cmake -DWEB_BUILD:INT=1 -DTK_CXX_EXTRA:STRING="-O3 -g" -S ../../ -G Ninja && ninja & pause