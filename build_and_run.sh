#!/usr/bin/env bash

PROJECT_NAME=$(basename $(pwd))

mkdir -p build && \
cd build && \
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -G Ninja --log-level=WARNING .. && \
cmake --build . && \
./$PROJECT_NAME
