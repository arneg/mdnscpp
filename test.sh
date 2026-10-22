#!/bin/sh
# Builds and runs the C++ tests in tests/. Extra arguments are passed to ctest.
#
# To run only the tests whose name contains a string, run the (single) test
# binary directly after building, e.g.
#   build-tests/mdnscpp-tests sockAddrToString
#
# Uses its own build directory (not "build") so that it does not interfere
# with build.sh, which only configures when "build" does not exist.
set -e

if [ ! -d build-tests ]; then
    echo "Running configure"
    cmake -S tests -B build-tests
fi
cmake --build build-tests -j4 --config Release
ctest --test-dir build-tests -C Release --output-on-failure "$@"
