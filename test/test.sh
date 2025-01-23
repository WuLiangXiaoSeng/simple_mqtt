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

case $1 in
    "msg_test")
        $BUILD_DIR/message_test
        ;;
    *)
        run_function msg_${1}_test
        ;;
esac