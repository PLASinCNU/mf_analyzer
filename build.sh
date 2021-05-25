#!/bin/sh

LLVM_DIR=/home/mok/project/HJ/llvm_install
LLVM_CMAKE_DIR=/home/mok/project/HJ/llvm_install/lib/cmake/llvm
export LLVM_DIR
export LLVM_CMAKE_DIR
current_directory=$(pwd)
build= "$current_directory/build"
if [ ! -d $build]; then
    echo "make build directory";
    mkdir build
else 
    echo "remove and remake build directory";
    rm -rf build
    mkdir build
fi

pushd build
cmake ../Transforms -DLLVM_CMAKE_DIR=/home/mok/project/HJ/llvm_install/lib/cmake/llvm
cmake --build .

