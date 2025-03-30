#!/bin/sh
if [ ! -d build ]; then
    echo "Running configure"
    cmake -Bbuild
fi
cmake --build build -j4
