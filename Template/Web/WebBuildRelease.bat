emcmake cmake -DEMSCRIPTEN=TRUE -DTK_CXX_EXTRA:STRING="-O3 -pthread -sPTHREAD_POOL_SIZE=4" -S ../Codes -G Ninja && ninja & set exit_code = %errorlevel% & exit exit_code