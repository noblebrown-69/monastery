#!/bin/bash
cd "$(dirname "$0")"
rm -rf build
mkdir build
cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
./Monastery
