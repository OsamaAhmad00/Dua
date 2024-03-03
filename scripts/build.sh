cmake -DCMAKE_BUILD_TYPE=Release -S .. -B ../build -Wno-dev
cmake --build ../build --target Dua -j 4
