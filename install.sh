#!/bin/bash

# Exit on error
set -e
# Print commands before execution
set -x

ROOT_DIR=$(pwd)
# install dependencies
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOT_DIR/dependencies/QGIS/build/lib

echo "Building dependencies... This may take a while"
if ! cmake --build . --parallel; then
    echo "Dependencies build failed!"
    exit 1
fi
echo "Dependencies build completed successfully"

cd $ROOT_DIR # return to root directory
# rm -rf build
mkdir -p build
cd build
cmake ..

echo "Building main project... This may take a while"
if ! cmake --build . --parallel; then
    echo "Main project build failed!"
    exit 1
fi
echo "Main project build completed successfully"
