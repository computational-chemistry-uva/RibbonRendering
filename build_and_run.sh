#!/usr/bin/env bash

#rm -f a.out
#echo "Compiling..."

#FLAGS="-Wall -g -std=c++20"
#SOURCE_FILES="*.cpp"
#time g++ $FLAGS $SOURCE_FILES -o a.out

#if [ -f "a.out" ]; then
#    time ./a.out
#	#valgrind --log-file="valgrind.log" ./a.out
#fi

mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja .. --log-level=WARNING && cmake --build . && ./biosplines
