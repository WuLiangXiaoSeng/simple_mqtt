#!/bin/bash
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:/root/code/mqtt/src/lib/

BUILD_DIR=$(pwd)/build

# file_exit()
# {
        
# }

run_function()
{
    $BUILD_DIR/$1
}


run_function $1