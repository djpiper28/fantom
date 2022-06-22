#!/bin/sh

astyle --style=kr src/**/*.c src/**/*.h tests/**/*.c tests/**/*.h
black .
cmake-format CMakeLists.txt > CMakeLists.txt.tmp && mv CMakeLists.txt.tmp CMakeLists.txt && rm CmakeLists.txt.tmp

