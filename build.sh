mkdir build
cd build
cmake -DTESTING=0 ..
make -j4
./wasm
