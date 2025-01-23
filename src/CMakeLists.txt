file(GLOB_RECURSE SRC_LIST "${PROJECT_SOURCE_DIR}/src/*.c")

add_library(simple_mqtt SHARED ${SRC_LIST})