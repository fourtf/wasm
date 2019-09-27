#!/usr/bin/env sh
mkdir build
cd build
cmake -DTESTING=1 ..
make -j4
./wasm
