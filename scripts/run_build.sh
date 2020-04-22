#!/bin/bash

mkdir -p "build" && cd "build" || exit 1
cmake .. -DCMAKE_BUILD_TYPE=Debug -DHW_ENABLE_ALL=ON
cmake --build .
