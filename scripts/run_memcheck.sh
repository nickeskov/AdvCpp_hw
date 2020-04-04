#!/bin/bash

mkdir -p "build" && cd "build" || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

valgrind --tool=memcheck --leak-check=full --leak-resolution=high --track-origins=yes ./hw
