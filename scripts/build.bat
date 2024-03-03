REM Make sure that cmake, ninja, and mt are in the path environment variable
REM Also, make sure that the environment variables CC and CXX are set to the location of clang-cl (usually C:/Program Files/LLVM/bin/clang-cl.exe)

cmake -DCMAKE_BUILD_TYPE=Release -G Ninja -S .. -B ../build -Wno-dev
cmake --build ../build --target Dua -j 4