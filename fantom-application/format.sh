#!/bin/sh

astyle --style=kr src/**/*.c src/*.c src/**/*.h src/*.h tests/**/*.c tests/*.c tests/**/*.h tests/*.h
black .
cmake-format CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt && rm -f CmakeLists.txt.tmp

