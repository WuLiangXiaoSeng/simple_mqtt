# cmake_minimum_required(VERSION 3.0)
project(simple_mqtt VERSION 0.1)

set(DIR_SRC ${PROJECT_SOURCE_DIR}/src)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/target/lib)

include_directories(
    ${DIR_SRC}
    ${DIR_SRC}/broker 
    ${DIR_SRC}/client 
    ${DIR_SRC}/log 
    ${DIR_SRC}/message
    ${DIR_SRC}/network
)


add_subdirectory(src)
# add_subdirectory(test)