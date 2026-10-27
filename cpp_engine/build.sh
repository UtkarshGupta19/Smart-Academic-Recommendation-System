#!/usr/bin/env bash
set -e

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
cd "$DIR"

echo "==> Building SARS C++17 Core Engine and Test Suite..."
mkdir -p bin

if [ -z "$CXX" ]; then
  if command -v clang++ >/dev/null 2>&1; then
    CXX=clang++
  elif command -v g++ >/dev/null 2>&1; then
    CXX=g++
  else
    CXX=c++
  fi
fi

CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -Iinclude"

echo "Compiler detected: $CXX"
echo "Compiling engine: $CXX $CXXFLAGS src/main.cpp -> bin/sars_engine"
$CXX $CXXFLAGS -o bin/sars_engine src/main.cpp

echo "Compiling test suite: $CXX $CXXFLAGS tests/test_suite.cpp -> bin/test_suite"
$CXX $CXXFLAGS -o bin/test_suite tests/test_suite.cpp

echo "Running automated test suite..."
./bin/test_suite

echo "==> C++ build & verification complete!"
