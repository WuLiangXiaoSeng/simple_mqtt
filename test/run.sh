#!/bin/bash
PWD=$(pwd)
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${PWD}/../src/lib/

BUILD_DIR=${PWD}/build

# file_exit()
# {
        
# }

run_function()
{
    $BUILD_DIR/$1
}


run_function $1