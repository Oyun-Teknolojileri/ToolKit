emcmake cmake -DTK_CXX_EXTRA:STRING="-O3 -g -pthread" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DWEB_BUILD:BOOL=1 -S ../Codes -G Ninja && ninja & set exit_code = %errorlevel% & exit exit_code