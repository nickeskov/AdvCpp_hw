#!/bin/bash

mkdir -p "build" && cd "build" || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .

valgrind -v --tool=memcheck --leak-check=full --show-leak-kinds=all \
  --leak-resolution=high --track-origins=yes ./hw
